source /home/cslt20/git/esp-idf/export.sh
source /home/cslt20/git/esp-matter/export.sh
export ESP_MATTER_DEVICE_PATH=$(pwd)/device/m5stampc3
idf.py set-target esp32c3
idf.py menuconfig
idf.py build