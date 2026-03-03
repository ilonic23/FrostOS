#include "ata.h"
#include "../cpu/ports.h"
#include <stdint.h>

// TODO: Rewrite a disk driver

void ata_read_sector(uint32_t lba, char *buffer) {
	port_byte_out(0x1F2, 1); // Sectors of the disk

	// Set LBA
	port_byte_out(0x1F3, (char)(lba & 0xFF));
	port_byte_out(0x1F4, (char)((lba >> 8) & 0xFF));
	port_byte_out(0x1F5, (char)((lba >> 16) & 0xFF));
	port_byte_out(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));

	/* после port_byte_out(0x1F6, ...) */
	// port_byte_in(0x1F7); port_byte_in(0x1F7); port_byte_in(0x1F7); port_byte_in(0x1F7);
	

	// Send command
	port_byte_out(0x1F7, 0x20);

	// Wait till ready
	while (port_byte_in(0x1F7) & 0x80);    // BSY
	while (!(port_byte_in(0x1F7) & 0x08)); // DRQ

	for (int i = 0; i < 256; i++) {
	    uint16_t w = port_word_in(0x1F0);
	    buffer[i * 2] = w & 0xFF;
	    buffer[i * 2 + 1] = w >> 8;
	}
}

void ata_write_sector(uint32_t lba, char *buffer) {
    // Количество секторов
    port_byte_out(0x1F2, 1);

    // Адрес LBA
    port_byte_out(0x1F3, (uint8_t)(lba & 0xFF));
    port_byte_out(0x1F4, (uint8_t)((lba >> 8) & 0xFF));
    port_byte_out(0x1F5, (uint8_t)((lba >> 16) & 0xFF));
    port_byte_out(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));

    // Команда записи
    port_byte_out(0x1F7, 0x30);

    // Ждем готовности
    while (port_byte_in(0x1F7) & 0x80);    // BSY
    while (!(port_byte_in(0x1F7) & 0x08)); // DRQ

    // Пишем 256 слов (512 байт)
    for (int i = 0; i < 256; i++) {
        uint16_t w = buffer[i*2] | (buffer[i*2+1] << 8);
        port_word_out(0x1F0, w);
    }
    

    // После записи можно проверить ошибки через 0x1F7
}

uint32_t ata_get_sectors() {
    port_byte_out(0x1F6, 0xA0); // мастер
    port_byte_out(0x1F2, 0);
    port_byte_out(0x1F3, 0);
    port_byte_out(0x1F4, 0);
    port_byte_out(0x1F5, 0);
    port_byte_out(0x1F7, 0xEC); // Identify

    while (port_byte_in(0x1F7) & 0x80);
    while (!(port_byte_in(0x1F7) & 0x08));

    uint16_t identify[256];
    for (int i = 0; i < 256; i++) identify[i] = port_word_in(0x1F0);

    uint32_t sectors = ((uint32_t)identify[61] << 16) | identify[60];
    return sectors;
}
