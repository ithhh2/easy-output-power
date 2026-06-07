#include "app_storage.h"

#include "app_config.h"
#include "app_state.h"
#include "power_profile.h"

#include "main.h"
#include "stm32f0xx_hal_flash.h"
#include "stm32f0xx_hal_flash_ex.h"

#define APP_STORAGE_MAGIC     0x50575231UL
#define APP_STORAGE_VERSION   1U
#define APP_STORAGE_ADDR      0x08007C00UL

typedef struct __attribute__((packed))
{
	uint32_t magic;
	uint16_t version;
	uint8_t setVolIndex;
	uint16_t protectValue;
	uint8_t reserved;
	uint32_t crc32;
} AppStorage_t;

static uint8_t save_pending = 0U;
static uint32_t save_due_ms = 0U;

static uint32_t storage_crc32(const uint8_t *data, uint32_t len)
{
	uint32_t crc = 0xFFFFFFFFUL;
	uint32_t i = 0U;
	uint32_t bit = 0U;

	for (i = 0U; i < len; i++)
	{
		crc ^= (uint32_t)data[i];
		for (bit = 0U; bit < 8U; bit++)
		{
			if ((crc & 1U) != 0U)
			{
				crc = (crc >> 1U) ^ 0xEDB88320UL;
			}
			else
			{
				crc >>= 1U;
			}
		}
	}

	return ~crc;
}

static uint8_t storage_record_valid(const AppStorage_t *record)
{
	uint32_t crc = 0U;

	if (record->magic != APP_STORAGE_MAGIC)
	{
		return 0U;
	}

	if (record->version != APP_STORAGE_VERSION)
	{
		return 0U;
	}

	if (record->setVolIndex >= POWER_TIER_COUNT)
	{
		return 0U;
	}

	if ((record->protectValue < PROTECT_CUR_MIN_MA) ||
	    (record->protectValue > PROTECT_CUR_MAX_MA))
	{
		return 0U;
	}

	crc = storage_crc32((const uint8_t *)record, (uint32_t)(sizeof(AppStorage_t) - sizeof(uint32_t)));
	if (crc != record->crc32)
	{
		return 0U;
	}

	return 1U;
}

static uint8_t storage_program_record(const AppStorage_t *record)
{
	FLASH_EraseInitTypeDef erase = {0};
	uint32_t page_error = 0U;
	uint32_t addr = APP_STORAGE_ADDR;
	const uint16_t *src = (const uint16_t *)record;
	uint32_t words = (sizeof(AppStorage_t) + 1U) / 2U;
	uint32_t i = 0U;

	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	erase.PageAddress = APP_STORAGE_ADDR;
	erase.NbPages = 1U;

	if (HAL_FLASH_Unlock() != HAL_OK)
	{
		return 0U;
	}

	if (HAL_FLASHEx_Erase(&erase, &page_error) != HAL_OK)
	{
		(void)HAL_FLASH_Lock();
		return 0U;
	}

	for (i = 0U; i < words; i++)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, src[i]) != HAL_OK)
		{
			(void)HAL_FLASH_Lock();
			return 0U;
		}
		addr += 2U;
	}

	(void)HAL_FLASH_Lock();
	return 1U;
}

void app_storage_load(void)
{
	const AppStorage_t *record = (const AppStorage_t *)APP_STORAGE_ADDR;
	AppState_t *state = app_state();

	if (storage_record_valid(record) == 0U)
	{
		return;
	}

	state->setVolIndex = record->setVolIndex;
	state->protectValue = record->protectValue;
}

void app_storage_request_save(void)
{
	save_pending = 1U;
	save_due_ms = HAL_GetTick() + STORAGE_SAVE_DEBOUNCE_MS;
}

static void app_storage_save_now(void)
{
	AppStorage_t record = {0};
	AppState_t *state = app_state();

	record.magic = APP_STORAGE_MAGIC;
	record.version = APP_STORAGE_VERSION;
	record.setVolIndex = state->setVolIndex;
	record.protectValue = state->protectValue;
	record.crc32 = storage_crc32((const uint8_t *)&record,
	                               (uint32_t)(sizeof(AppStorage_t) - sizeof(uint32_t)));
	(void)storage_program_record(&record);
}

void app_storage_tick(void)
{
	if (save_pending == 0U)
	{
		return;
	}

	if ((int32_t)(HAL_GetTick() - save_due_ms) < 0)
	{
		return;
	}

	save_pending = 0U;
	app_storage_save_now();
}
