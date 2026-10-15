#include <string.h>
#include "tusb.h"

#define USB_VID 0x2E8A /* Raspberry Pi */
#define USB_PID 0x000A /* Pico SDK CDC */

enum { ITF_NUM_CDC = 0, ITF_NUM_CDC_DATA, ITF_NUM_TOTAL };

#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT   0x02
#define EPNUM_CDC_IN    0x82
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

static tusb_desc_device_t const desc_device = {
	.bLength            = sizeof(tusb_desc_device_t),
	.bDescriptorType    = TUSB_DESC_DEVICE,
	.bcdUSB             = 0x0200,
	.bDeviceClass       = TUSB_CLASS_MISC,
	.bDeviceSubClass    = MISC_SUBCLASS_COMMON,
	.bDeviceProtocol    = MISC_PROTOCOL_IAD,
	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
	.idVendor           = USB_VID,
	.idProduct          = USB_PID,
	.bcdDevice          = 0x0100,
	.iManufacturer      = 1,
	.iProduct           = 2,
	.iSerialNumber      = 3,
	.bNumConfigurations = 1,
};

static uint8_t const desc_configuration[] = {
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
	TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8,
			   EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

static char const *const string_desc[] = {
	NULL, /* language, handled below */
	"Raspberry Pi",
	"PIO UART Bridge",
	"0001",
	"PIO UART",
};

uint8_t const *tud_descriptor_device_cb(void)
{
	return (uint8_t const *)&desc_device;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
	(void)index;
	return desc_configuration;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	static uint16_t desc_str[32];
	size_t len;

	(void)langid;
	if (index == 0) {
		desc_str[1] = 0x0409;
		len = 1;
	} else {
		char const *str;

		if (index >= sizeof(string_desc) / sizeof(string_desc[0]))
			return NULL;
		str = string_desc[index];
		len = strlen(str);
		if (len > 31)
			len = 31;
		for (size_t i = 0; i < len; i++)
			desc_str[1 + i] = str[i];
	}
	desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * len + 2));
	return desc_str;
}
