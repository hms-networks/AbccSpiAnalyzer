import csv
import matplotlib.pyplot as pyplot

FILE_LIST = [
    #'process_data_125MSps.csv',
    'process_data_plug_pulled3_125MSps.csv'
]

CSV_DELIMITER = '\t'
#OUTPUT_PD_VARIABLE_BIT_OFFSET = 128
#OUTPUT_PD_VARIABLE_BIT_SIZE = 16
#INPUT_PD_VARIABLE_BIT_OFFSET = 528
#INPUT_PD_VARIABLE_BIT_SIZE = 16

# EIP, Module 1, Channel 4
OUTPUT_PD_VARIABLE_BIT_OFFSET = 40 * 8
OUTPUT_PD_VARIABLE_BIT_SIZE = 16
INPUT_PD_VARIABLE_BIT_OFFSET = 40 * 8
INPUT_PD_VARIABLE_BIT_SIZE = 16

def extract_value( csv_row_entry, bit_offset, bit_size ):
    byte_offset = int(bit_offset / 8)
    num_bytes = int(((bit_offset % 8) + bit_size + 7)/8)
    value = 0
    for i in range(0,num_bytes):
        strValue = csv_row_entry["Process Data {0}".format(byte_offset + i)]
        try:
            value = value + (int(strValue) << (i*8))
        except:
            value = value + (int(strValue,16) << (i*8))
            pass

    value = value >> (bit_offset % 8)
    if value & 0x80000000:
        value = -1 * (value ^ 0xFFFFFFFF) + 1

    return value

for f in FILE_LIST:
    mosi_sig = []
    tim_mosi = []
    nw_tim_mosi = []

    miso_sig = []
    tim_miso = []
    nw_tim_miso = []
    op_state_markers = []
    so_state_markers = []

    with open(f, mode='r') as csv_file:
        is_operational = 0
        csv_reader = csv.DictReader(csv_file, delimiter=CSV_DELIMITER)
        line_count = 0
        for row in csv_reader:
            sample_time = float(row["Time [s]"])
            network_time = float(row["Network Time"])
            # Only plot sample points from SPI packets that contain no errors
            if row["Error Event"] == "":
                if row["Channel"] == "MOSI":
                    value = extract_value(row, INPUT_PD_VARIABLE_BIT_OFFSET, INPUT_PD_VARIABLE_BIT_SIZE)
                    mosi_sig.append(value)
                    tim_mosi.append(sample_time)
                    nw_tim_mosi.append(network_time)

                elif row["Channel"] == "MISO":
                    value = extract_value(row, OUTPUT_PD_VARIABLE_BIT_OFFSET, OUTPUT_PD_VARIABLE_BIT_SIZE)
                    miso_sig.append(value)
                    tim_miso.append(sample_time)
                    nw_tim_miso.append(network_time)
                if "PROCESS_ACTIVE" in row["Anybus State"]:
                    if not is_operational:
                        is_operational = 1
                        op_state_markers.append(sample_time)
                else:
                    if is_operational:
                        is_operational = 0
                        so_state_markers.append(sample_time)

        miso_pkt_rate = int(len(tim_miso) / (tim_miso[-1] - tim_miso[0]))
        mosi_pkt_rate = int(len(tim_mosi) / (tim_mosi[-1] - tim_mosi[0]))

        #pyplot.subplot(211)
        lines = pyplot.plot(tim_mosi, mosi_sig, 'b.-',
                            tim_miso, miso_sig, 'r.-')

        pyplot.setp(lines[0], linewidth=0.25)
        pyplot.setp(lines[1], linewidth=0.25)

        for i in op_state_markers:
            pyplot.axvline(x=i, color='g', linestyle=':')

        for i in so_state_markers:
            pyplot.axvline(x=i, color='r', linestyle=':')

        pyplot.legend(('MOSI (AI) @ ~' + str(mosi_pkt_rate) + ' (updates/sec)',
                       'MISO (AO) @ ~' + str(miso_pkt_rate) + ' (updates/sec)'),
                      loc='upper right', shadow=True)

        pyplot.title('Process Data (UINT16)')
        pyplot.ylabel('Process Data Value')
        pyplot.xlabel('Time[s]')
        pyplot.grid(True)

        #pyplot.subplot(212)
        #lines = pyplot.plot(nw_tim_mosi, mosi_sig, 'b.-',
                            #nw_tim_miso, miso_sig, 'r.-')

        #pyplot.setp(lines[0], linewidth=0.25)
        #pyplot.setp(lines[1], linewidth=0.25)

        #pyplot.legend(('MOSI (Analog Input)', 'MISO (Analog Output)',),
                      #loc='upper right', shadow=True)

        #pyplot.title('Process Data (UINT16) @ ~' + str(pkt_rate) + ' (packets/sec)')
        #pyplot.ylabel('Process Data Value')
        #pyplot.xlabel('Network Time')
        #pyplot.grid(True)

        #pyplot.subplots_adjust(top=0.92, bottom=0.10, left=0.15, right=0.95, hspace=0.45,
                               #wspace=0.35)

        pyplot.show()
