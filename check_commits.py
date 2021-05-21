#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""
Script to validate each commit between two reference points (inclusive).
Script will abort if starting point is "behind" ending point (validation works
backwards). On validation error the corresponding exit code will be displayed.

Run "python .\check_commits.py --help" for command documentation.

Example Usage:
❯ python .\check_commits.py -b "<CMD_LINE_EXPRESSION>" -c 50 -s "dev" -e "origin/master"
Creating temporary branch: check_commit
Checkout temporary branch: check_commit
Validating 16 commits
-- Start: 96a7870b32c879ae1751041bbd17ecc4d836f0ba
-- End: f202b99c03096b18e56f22ab6183fbc56cf9a908
* Validating 96a7870b32 ... SKIP (No Sources Modified)
* Validating 00b9224e9d ... PASS
* Validating 8d00dac308 ... SKIP (No Sources Modified)
* Validating 9e3462af31 ... SKIP (No Sources Modified)
* Validating 55f07777ed ... SKIP (No Sources Modified)
* Validating 035856079b ... SKIP (No Sources Modified)
* Validating 3b78794a82 ... PASS
* Validating d690326bea ... PASS
* Validating d6051322bb ... PASS
* Validating c6c90f7ac5 ... FAIL (exit_code = 1)
* Validating e29acc952a ... FAIL (exit_code = 1)
* Validating 77456eb339 ... PASS
* Validating c84d9ed57b ... PASS
* Validating 5689b2d95c ... PASS
* Validating e93baa3929 ... SKIP (No Sources Modified)
* Validating f202b99c03 ... SKIP (No Sources Modified)
Checking out: dev
Deleting temporary branch: check_commit
"""

import argparse
import os
import subprocess
import signal
from typing import Any
import git

EXTENSIONS_TO_CHECK_CS = [".cs", ".json", ".resx", ".sln", ".csproj"]
EXTENSIONS_TO_CHECK_C = [".c", ".cpp", ".h", ".hpp"]

EXTENSIONS_TO_CHECK = EXTENSIONS_TO_CHECK_C
DEFAULT_TEMP_BRANCH_NAME = "check_commit"

abort = False


def _signal_handler(sig, frame) -> None:
    '''
    Handles aborting the process.
    '''

    global abort
    abort = True


def _cleanup(git_repo, original_branch_name, temp_branch_name) -> None:
    '''
    Performs clean up before script exits. Restoring back to the
    original branch prior to script execution.

    Parameters:
    -----------
    git_repo: The object instance of the git repository to check.
    original_branch_name: The original branch name to restore to.
    temp_branch_name: The temporary branch name to delete.
    '''

    print(f"Checking out: {original_branch_name}")
    git_repo.heads[original_branch_name].checkout()

    print(f"Deleting temporary branch: {temp_branch_name}")
    git_repo.delete_head(temp_branch_name)


def _check_commits(git_repo: object, args: object) -> bool:
    '''
    Checks if the specified commit history range passes validation checks.

    Parameters:
    -----------
    git_repo: The object instance of the git repository to check.
    args: The user specified command line arguments.

    Returns:
    --------
    True if all commits in specified range pass validation;
    otherwise, returns False.
    '''

    active_branch_name = git_repo.active_branch.name

    build_command = args.build_command.split()

    if args.temp_branch_name == None:
        temp_branch_name = DEFAULT_TEMP_BRANCH_NAME
    else:
        temp_branch_name = args.temp_branch_name

    if args.commit_count <= 0:
        print("In order to run this script, the commit count must be greater than 0.")
        return False

    if git_repo.is_dirty():
        print("In order to run this script, the repo must not contain uncommitted changes to tracked files.")
        return False

    if git_repo.head.is_detached:
        print("In order to run this script, GIT HEAD must not be detached.")
        return False

    if active_branch_name == temp_branch_name:
        print(
            f'In order to run this script, the currently active branch must not be "{temp_branch_name}".')
        return False

    try:
        temp_branch = git_repo.refs[temp_branch_name]
        git_repo.delete_head(temp_branch)
    except IndexError:
        pass

    print(f"Creating temporary branch: {temp_branch_name}")
    temp_branch = git_repo.create_head(temp_branch_name)

    print(f"Checkout temporary branch: {temp_branch_name}")
    temp_branch.checkout()

    if args.start_commit != None:
        start_commit = git_repo.commit(args.start_commit).hexsha
    else:
        # Default to using current branch head as the starting point
        start_commit = temp_branch.commit.hexsha

    if args.end_commit != None:
        end_commit = git_repo.commit(args.end_commit).hexsha
    else:
        # Default to the user-specified commit_count for the ending point.
        git_repo.active_branch.commit = git_repo.commit(start_commit)

        total_commits = int(git_repo.git.rev_list('--count', 'HEAD'))

        if args.commit_count > total_commits:
            print(
                f"Commit count limit reached. Limiting to {total_commits} commits")
            args.commit_count = total_commits

        end_commit = git_repo.commit(f'HEAD~{args.commit_count-1}').hexsha

    if start_commit == end_commit:
        commit_list = [start_commit]
    else:
        commit_list = git_repo.git.rev_list(
            start_commit, '--not', end_commit, '--boundary')
        commit_list = commit_list.replace('-', '').split('\n')

    if len(commit_list) > args.commit_count:
        print(
            f"Commit count limit reached. Limiting to {args.commit_count} commits")
        commit_list = commit_list[0:args.commit_count]

    print(f"Validating {len(commit_list)} commits")
    print(f"-- Start: {start_commit}")
    print(f"-- End: {end_commit}")

    for i in commit_list:
        if abort:
            print("Aborting!")
            break

        git_repo.git.reset(i, "--hard")

        print(
            f"* Validating {git_repo.active_branch.commit.hexsha[0:10]} ... ", end='')

        if _sources_modified(git_repo.active_branch.commit):
            exit_code = subprocess.call(build_command,
                                        stdout=subprocess.DEVNULL,
                                        stderr=subprocess.DEVNULL)

            if exit_code != 0:
                print(f"FAIL (exit_code = {exit_code})")
            else:
                print("PASS")
        else:
            print("SKIP (No Sources Modified)")

    _cleanup(git_repo, active_branch_name, temp_branch_name)

    return True


def _sources_modified(commit: object) -> bool:
    '''
    Reports whether relevant sources have been modified that justify
    revalidation such as building the project.

    Parameters:
    -----------
    commit: The commit object to inspect what sources have been modified.

    Returns:
    --------
    True if an relevant sources have been modified;
    otherwise, returns False.
    '''

    for modified_file in list(commit.stats.files):
        _, file_extension =  os.path.splitext(modified_file)

        if any(ext in file_extension for ext in EXTENSIONS_TO_CHECK):
            return True

    return False


def _handle_arguments() -> object:
    '''
    Command line argument handler.

    Returns:
    --------
    The parsed command line arguments.
    '''

    parser = argparse.ArgumentParser()
    parser.add_argument('-b', '--build-command',
                        dest='build_command',
                        required=True,
                        type=str,
                        help='The command to execute in command line for verification.')
    parser.add_argument('-c', '--commit-count',
                        dest='commit_count',
                        required=True,
                        type=int,
                        help='Number of commits to verify (starting at HEAD). If using "-s" and "-e", this option acts as a limiter.')
    parser.add_argument('-t', '--temp-branch-name',
                        dest='temp_branch_name',
                        type=str,
                        help='Temporary branch name used to traverse the commit history (Default="check_commit").')
    parser.add_argument('-s', '--start-commit',
                        dest='start_commit',
                        type=str,
                        help='Starting point, must be a commit ahead of "-e". Value may be a branch name or commit SHA.')
    parser.add_argument('-e', '--end-commit',
                        dest='end_commit',
                        type=str,
                        help='End point, must be a commit behind "-s". Value may be a branch name or commit SHA. If this option is omitted "-c" defines the stopping point. Please note: merge commits may not work as expected when encountered.')
    return parser.parse_args()


if __name__ == "__main__":
    signal.signal(signal.SIGINT, _signal_handler)
    args = _handle_arguments()
    git_repo = git.Repo()
    result = _check_commits(git_repo, args)

    if not result:
        exit(1)
