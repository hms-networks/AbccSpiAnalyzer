#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""
Produces a ZIP-file release and optionally creates a tag on the GIT repository.
The released ZIP-file will be appended with the plugin's version if:
- A tag was created.
- A tag exists on HEAD.
Otherwise, the released ZIP-file will be appended with the SHA of the current
commit.
If the GIT repo's HEAD contains uncommitted changes, the release filename will
be appended with "dirty" regardless of whether or not the changes are functional
or not.
"""

import os
import re
import glob
import git
import zipfile

from datetime import date, datetime, timezone

# Source entries must follow glob conventions. Destination entries must match
# scaling of the source entry. In other words the destination should not provide
# an explicit file name if the source may yield multiple files.
FILE_LIST = [
    {"src": "./doc/AbccSpiAnalyzer_Plugin_Quick_Start_Guide.pdf", "dst": "doc/"},
    {"src": "./doc/AdvancedSettings.xml", "dst": "doc/"},
    {"src": "./plugins/Linux32/libAbccSpiAnalyzer.so", "dst": "plugins/Linux32/"},
    {"src": "./plugins/Linux64/libAbccSpiAnalyzer64.so", "dst": "plugins/Linux64/"},
    {"src": "./plugins/OSX/libAbccSpiAnalyzer.dylib", "dst": "plugins/OSX/"},
    {"src": "./plugins/Win32/AbccSpiAnalyzer.dll", "dst": "plugins/Win32/"},
    {"src": "./plugins/Win64/AbccSpiAnalyzer64.dll", "dst": "plugins/Win64/"},
    {"src": "./CHANGELOG.md", "dst": ""},
    {"src": "./KnownLimitations.md", "dst": ""},
    {"src": "./LICENSE.md", "dst": ""},
]

# Path where the released ZIP-file will be written to.
RELEASE_PATH = "./release/"

# Base name of the released ZIP-file, additional info will be appended for
# version identification purposes as needed.
BASE_ZIP_FILENAME = "AbccSpiAnalyzer"

# The ZIP-file extension.
ZIP_EXT = ".zip"


# The file where version metadata is stored.
VERSION_INFO_FILE = "./source/AbccSpiMetadata.h"

# The regular expressions to search each line fore to collect version info.
FILEVERSION_YEAR_TAG = r"#define\s+FILEVERSION_YEAR\s+(\d+)"
FILEVERSION_MONTH_TAG = r"#define\s+FILEVERSION_MONTH\s+(\d+)"
FILEVERSION_DAY_TAG = r"#define\s+FILEVERSION_DAY\s+(\d+)"
FILEVERSION_BUILD_TAG = r"#define\s+FILEVERSION_BUILD\s+(\d+)"


class ReleaseVersion:
    """
    A class to define release version conventions used in this repository.
    """

    year: int
    month: int
    day: int
    build: int


    def __init__(self, year: int =0, month: int =0, day: int =0, build: int =0):
        self.year = year
        self.month = month
        self.day = day
        self.build = build


    def set_version_from_list(self, version: list) -> bool:
        """
        Set the version using a list of four integers.

        Parameters
        ----------
        version : list
            List of four integers in the format: [year, month, day, build].

        Returns
        ----------
        Returns True if the version was successfully set; otherwise, the
        set request was rejected due to a format issue and False is returned.
        """

        set_success = False
        year = version[0]
        month = version[1]
        day = version[2]
        build = version[3]

        # Perform very basic plausibility check
        if (len(version) == 4) and (year > 0) or (month > 0) or (day > 0) or (build > 0) or (month <= 12) or (day <= 31):
            self.year = year
            self.month = month
            self.day = day
            self.build = build
            set_success = True

        return set_success


    def set_version_from_tag_name(self, tag_name: str) -> bool:
        """
        Set the version using a GIT repository tag name.

        Parameters
        ----------
        tag_name : str
            String in the format of REL_YYYY_MM_DD_B.

        Returns
        ----------
        Returns True if the version was successfully set; otherwise, the
        set request was rejected due to a format issue and False is returned.
        """

        set_success = False
        version_strings = re.search(r'^REL_[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$', tag_name)

        if hasattr(version_strings, 'group'):
            version_list = list(map(int, version_strings.group(0).split("_")[1].split('.')))
            self.set_version_from_list(version_list)
            set_success = True

        return set_success


    def tag_format(self) -> str:
        """
        Returns the version as a string using the tag format of this repository.
        """

        return "REL_" + f"{self.year}.{self.month}.{self.day}.{self.build}"


    def list_format(self) -> list:
        """
        Returns a list of four integers in the form [year, month, day, build].
        """

        return [self.year, self.month, self.day, self.build]


    def str_format(self) -> str:
        """
        Returns a version string meant for printing to console.
        """

        return f"v{self.year}.{self.month}.{self.day}.{self.build}"


    def file_format(self) -> str:
        """
        Returns a version string meant for the version component of the release
        filename.
        """

        return f"{self.year:04d}{self.month:02d}{self.day:02d}{self.build}"


    def is_equal(self, ref_version: object) -> bool:
        """
        This routine compares `this` version against ref_version, both of which
        are in the form of YYYY.MM.DD.B which represents a date with an
        incremental build number which resets to 1 on a new release date.

        Parameters
        ----------
        ref_version : object
            A ReleaseVersion to compare `this` ReleaseVersion against.

        Returns
        ----------
        Returns True if the `this` ReleaseVersion is equal to the ref_version;
        otherwise, returns False.
        """

        return (ref_version != None) and (ref_version.str_format() == self.str_format())


    def is_newer_than(self, ref_version: object) -> bool:
        """
        This routine compares `this` version against ref_version, both of which
        are in the form of YYYY.MM.DD.B which represents a date with an
        incremental build number which resets to 1 on a new release date.

        Parameters
        ----------
        ref_version : object
            A ReleaseVersion to compare `this` ReleaseVersion against.

        Returns
        ----------
        Returns True if the `this` ReleaseVersion is newer than the ref_version;
        otherwise, returns False.
        """

        this_version_is_newer = False

        if ref_version == None:
            this_version_is_newer = True
        else:
            for i in range(4):
                if self.list_format()[i] > ref_version.list_format()[i]:
                    this_version_is_newer = True
                    break

        return this_version_is_newer


    def next_version(self) -> object:
        """
        Computes a valid version identifier based on `this` version. This method
        should be used on the `last tagged version` to get a valid new version.

        The versioning rules are as follows:
        If the last tag and current date are the same, increment the build number;
        If the last tag and current date differ, reset build to 1 and use current
        date.

        Returns
        ----------
        A new ReleaseVersion object indicating the next version.
        """

        today = date.today()
        this_date = date(self.year, self.month, self.day)
        new_version = ReleaseVersion(today.year, today.month, today.day, 1)

        if today == this_date:
            new_version.build = self.build + 1

        return new_version


def _get_git_head_tag(git_repo: object) -> object:
    """
    This routine handles retrieval of the first tag reported in HEAD.

    Parameters
    ----------
    git_repo : object
        The GIT repository (git.Repo) that this project and file exists within.

    Returns
    ----------
    A git.Repo tag object; otherwise, if no tag exists, this routine will
    return None.
    """

    head_sha = git_repo.head.commit.hexsha

    for tag in git_repo.tags:
        if tag.object.hexsha == head_sha:
            return tag

    return None


def _get_last_tagged_commit(git_repo: object) -> list:
    """
    This routine handles retrieval of the last GIT repo's tagged commit based
    on the formatted version info of the tag name.

    Parameters
    ----------
    git_repo : object
        The GIT repository (git.Repo) that this project and file exists within.

    Returns
    ----------
    newest_tag_version: object
        A ReleaseVersion object indicating the last tagged version.
        Returns None, if no tags exist with the expect version string format.

    newest_tag: object
        A git.Repo commit object of the last tagged version.
        Returns None, if no tags exist with the expect version string format.
    """

    newest_tag = None
    newest_tag_version = None
    version = ReleaseVersion()

    for tag in git_repo.tags:
        if version.set_version_from_tag_name(tag.name):
            if version.is_newer_than(newest_tag_version):
                newest_tag_version = version
                newest_tag = tag

    return [newest_tag_version, newest_tag]


def _request_tag_name(tag_version: object) -> object:
    """
    Prompts the user for a GIT tag name. Basic validation is performed to
    ensure consistent tag version format.

    Returns
    ----------
    A ReleaseVersion object indicating the user-accepted tag version.
    """

    valid_tag_name = False

    while not valid_tag_name:
        new_tag_name = input(f"Tag name [{tag_version.tag_format()}]: ").strip()

        if len(new_tag_name) > 0:
            if tag_version.set_version_from_tag_name(new_tag_name):
                valid_tag_name = True
            else:
                print("Invalid tag name. Must follow 'REL_YYYY.MM.DD.B' format")
        else:
            valid_tag_name = True

    return tag_version


def _create_git_tag(git_repo: object, tag_version: object) -> bool:
    """
    Creates a tag in the GIT repository when specified by the user.
    Reports if a tag was created or not.

    Parameters
    ----------
    git_repo : object
        The GIT repository (git.Repo) that this project and file exists within.
    tag_version: object
        A candidate GIT tag version to use. Function will prompt user for confirmation.

    Returns
    ----------
    True if a GIT tag was created; otherwise, returns False.
    """

    new_tag_created = False
    response = input("Create a new Tag Y/[N]? ").strip()

    if response == "Y" or response == "y":
        tag_version = _request_tag_name(tag_version)

        print("- Creating TAG on HEAD...")
        git_repo.create_tag(tag_version.tag_format())

        print("- New tag created: " + tag_version.tag_format())
        new_tag_created = True

    return [new_tag_created, tag_version]


def _zip_glob(commit_date: object, zip_file: object, file_entry: dict) -> None:
    """
    Handles adding the file(s) specified in the source (src) file_entry to the
    specified destination (dst).

    Parameters
    ----------
    commit_date: object
        The date of the commit the release is based on. This is only used to
        help notify the user of potentially stale library files.
    zip_file : object
        ZipFile object used to add the specified file_entry to.
    file_entry: dict
        A dictionary indicating a glob source file(s) and destination within
        the zip_file to write to.

    Returns
    ----------
    None
    """

    lib_extensions = ('.so', '.dll', '.dylib')

    files = glob.glob(file_entry["src"])

    if len(files) == 0:
        print(f'- WARNING: No matches for "{file_entry["src"]}".')

    for src_file in files:
        filename = os.path.basename(src_file)
        dest_file = file_entry["dst"] + filename
        modification_date = datetime.fromtimestamp(
            os.stat(src_file).st_mtime, tz=timezone.utc)

        modification_date_str = modification_date.strftime("%x %X")

        print(f"- ({modification_date_str} UTC) " + src_file)

        if modification_date < commit_date and filename.endswith(lib_extensions):
            commit_date_str = commit_date.strftime("%x %X")
            print(f"-- WARNING: Library file is older than commit date ({commit_date_str} UTC).")
            print("")

        zip_file.write(src_file, dest_file)


def _make_zip_release(commit_date: object, filepath: str) -> None:
    """
    This routine handles copying files specified in FILE_LIST into a ZIP-file.

    Parameters
    ----------
    commit_date: object
        The date of the commit the release is based on. This is only used to
        help notify the user of potentially stale library files.
    filepath : str
        Filename and path indicating where to write the ZIP-file.


    Returns
    ----------
    None
    """

    path = os.path.dirname(filepath)

    if not os.path.exists(path):
        os.makedirs(path)

    zip_file = zipfile.ZipFile(filepath, 'w', zipfile.ZIP_DEFLATED)

    print("Adding files to ZIP archive...")

    for file_entry in FILE_LIST:
        _zip_glob(commit_date, zip_file, file_entry)

    zip_file.close()


def _get_version_from_file() -> object:
    """
    Parses the C++ source file specified by VERSION_INFO_FILE for version
    information. The format of the version information is specified by:
    FILEVERSION_YEAR_TAG, FILEVERSION_MONTH_TAG, FILEVERSION_DAY_TAG, and
    FILEVERSION_BUILD_TAG. In general, version info should not span multiple
    lines.

    Returns:
        The ReleaseVersion of the source file. If parsing fails to obtain
        one or more components of the version, None will be returned.
    """

    valid_version = False
    file_version = None
    year = None
    month = None
    day = None
    build = None

    file_lines = open(VERSION_INFO_FILE, 'r').readlines()

    for line in file_lines:
        if year == None:
            regex_result = re.search(FILEVERSION_YEAR_TAG, line)
            if hasattr(regex_result, 'group'):
                year = regex_result[1]

        if month == None:
            regex_result = re.search(FILEVERSION_MONTH_TAG, line)
            if hasattr(regex_result, 'group'):
                month = regex_result[1]

        if day == None:
            regex_result = re.search(FILEVERSION_DAY_TAG, line)
            if hasattr(regex_result, 'group'):
                day = regex_result[1]

        if build == None:
            regex_result = re.search(FILEVERSION_BUILD_TAG, line)
            if hasattr(regex_result, 'group'):
                build = regex_result[1]

        valid_version = (
            year != None and
            month != None and
            day != None and
            build != None)

        if valid_version:
            break;

    if valid_version:
        file_version = ReleaseVersion()
        file_version.set_version_from_list(
            list(map(int, [year, month, day, build])))

    return file_version


def make_release() -> None:
    """
    Main routine that performs the release process.
    """

    print("Accessing GIT repository...")

    git_repo = git.Repo('.')

    dirty = git_repo.is_dirty()
    mainline = git_repo.head.commit.hexsha == git_repo.remotes.origin.refs.master.commit.hexsha

    if not mainline:
        print("")
        print("WARNING: The GIT repository's HEAD is not on origin/master commit.")
        print("         Releases are typically only done on master.")

    if dirty:
        print("")
        print("WARNING: The GIT repository's working directory has uncommitted changes.")
        print("         Please be sure that these changes do not affect the release state.")
        dirty = True

    if dirty or not mainline:
        print("")

    print("Collecting version information...")

    source_file_version = _get_version_from_file()

    if source_file_version != None:
        print("- Source: " + source_file_version.str_format())
    else:
        print("ERROR: The source file version could not be parsed!")
        print("       The issue must be resolved to continue with the release process.")
        print("")
        input("Press [ENTER] to continue.")
        return

    [last_tagged_version, last_tag] = _get_last_tagged_commit(git_repo)

    if last_tagged_version != None:
        print(f"- Last tag: {last_tagged_version.str_format()}")
    else:
        print(f"- Last tag: NONE")
        last_tagged_version =  ReleaseVersion()

    tagged_head = _get_git_head_tag(git_repo)

    tag_created = False

    # Only ask to create a new tag if one does not already exist.
    # If a tag exists then this release is targetting that tag.
    # If no tag exists and the user does not opt to create a tag
    # then the commit hash will be used. For simplicity, "dirty" will
    # be appended to any release that has un-staged changes regardless
    # of whether the changes impact function or not.

    if tagged_head != None:
        tagged_head_version = ReleaseVersion()
        tagged_head_version.set_version_from_tag_name(tagged_head.name)
        print(f"- HEAD tag: {tagged_head_version.str_format()}")
    else:
        print(f"- HEAD tag: NONE")

        if source_file_version.is_equal(last_tagged_version):
            print("")
            print("WARNING: Source file version has not changed since last tagged release.")
            print("         When tagging a new release it is expected a new version is used.")
            print("         The option to tag this release will be skipped.")
            print("")
        else:
            expected_tag_version = last_tagged_version.next_version()

            if not source_file_version.is_equal(expected_tag_version):
                print("")
                print("WARNING: The source file version does not match the current date.")
                print("         Typically when creating a new tag, the source file's date.")
                print("         should match the tag date.")
                print("")

            new_tagged_version = source_file_version

            [tag_created, new_tagged_version] = _create_git_tag(git_repo, new_tagged_version)

    release_filepath = RELEASE_PATH

    if tag_created:
        release_filepath += f"{BASE_ZIP_FILENAME}_{new_tagged_version.file_format()}"
    else:
        if tagged_head != None:
            release_filepath += f"{BASE_ZIP_FILENAME}_{tagged_head_version.file_format()}"
        else:
            # Use first 8 characters of commit hash for filename version details
            head_sha = git_repo.head.object.hexsha
            release_filepath += f"{BASE_ZIP_FILENAME}_SHA_{head_sha[:8].upper()}"

    if dirty:
        release_filepath += "_dirty"

    release_filepath += ZIP_EXT

    print("Creating ZIP archive...")

    commit_date = datetime.fromtimestamp(
        git_repo.head.commit.committed_date, tz=timezone.utc)

    _make_zip_release(commit_date, release_filepath)

    print("Created ZIP archive: " + release_filepath)
    print("Done.")


if __name__ == '__main__':
    make_release()
