
esptool.py --chip esp32s3 merge_bin \
  -o all.bin \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size 32MB \
  0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/ota_data_initial.bin 0x20000 build/GK87-Screen.bin
  