/*
 * Copyright (c) 2012, 2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* standard library header */
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/time.h>
#include <glib.h>

/* local header */
#include "smartcard-types.h"
#include "Debug.h"
#include "TerminalInterface.h"
#include "NFCTerminal.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#define ASYNC

using namespace smartcard_service_api;

static const char *se_name = "eSE";

/* below functions will be called when dlopen or dlclose is called */
void __attribute__ ((constructor)) lib_init()
{
}

void __attribute__ ((destructor)) lib_fini()
{
}

/* below three functions must be implemented */
extern "C"
{
EXPORT_API const char *get_name()
{
	return se_name;
}

EXPORT_API void *create_instance()
{
	return (void *)NFCTerminal::getInstance();
}

EXPORT_API void destroy_instance(void *instance)
{
	NFCTerminal *inst = (NFCTerminal *)instance;

	if (inst == NFCTerminal::getInstance())
	{
		inst->finalize();
	}
	else
	{
		_ERR("instance is invalid : getInstance [%p], instance [%p]",
				NFCTerminal::getInstance(), instance);
	}
}
}

namespace smartcard_service_api
{
	NFCTerminal::NFCTerminal() : Terminal(), seHandle(NULL),
		present(false), referred(0)
	{
		name = (char *)se_name;

		initialize();
	}

	NFCTerminal *NFCTerminal::getInstance()
	{
		static NFCTerminal instance;

		return &instance;
	}

	NFCTerminal::~NFCTerminal()
	{
		finalize();
	}

	bool NFCTerminal::initialize()
	{
		int ret;

		if (initialized == true)
			return initialized;

		ret = nfc_manager_initialize_sync();
		if (ret == NFC_ERROR_NONE)
		{
			initialized = true;

			if (open() == true) {
				present = true;
				close();
			}
		}
		else
		{
			_ERR("net_nfc_initialize failed [%d]", ret);
		}

		return initialized;
	}

	void NFCTerminal::finalize()
	{
		int ret;

		if (isClosed() == false) {
			/* close now */
			ret = nfc_se_close_secure_element_internal(seHandle);
			if (ret == NFC_ERROR_NONE) {
				seHandle = NULL;
				closed = true;
				referred = 0;
			} else {
				_ERR("nfc_se_close_secure_element_internal failed [%d]", ret);
			}
		}

		ret = nfc_manager_deinitialize();
		if (ret == NFC_ERROR_NONE) {
			initialized = false;
		} else {
			_ERR("nfc_manager_deinitialize failed [%d]", ret);
		}
	}

	bool NFCTerminal::open()
	{
		int ret;

		_BEGIN();

		if (isInitialized()) {
			if (referred == 0) {
				ret = nfc_se_open_secure_element_internal(NFC_SE_TYPE_ESE,
					&seHandle);
				if (ret == NFC_ERROR_NONE) {
					closed = false;
					referred++;
				} else {
					_ERR("nfc_se_open_secure_element_internal failed [%d]", ret);
				}
			} else {
				referred++;
			}

			_DBG("reference count [%d]", referred);
		}

		_END();

		return (isClosed() == false);
	}

	void NFCTerminal::close()
	{
		int ret;

		_BEGIN();

		if (isInitialized())
		{
			if (referred <= 1) {
				ret = nfc_se_close_secure_element_internal(seHandle);
				if (ret == NFC_ERROR_NONE) {
					seHandle = NULL;
					closed = true;
					referred = 0;
				} else {
					_ERR("nfc_se_close_secure_element_internal failed [%d]", ret);
				}
			} else {
				referred--;
			}

			_DBG("reference count [%d]", referred);
		}

		_END();
	}

	int NFCTerminal::transmitSync(const ByteArray &command, ByteArray &response)
	{
		int rv = SCARD_ERROR_NOT_INITIALIZED;

		_BEGIN();

		if (isClosed() == false)
		{
			if (command.size() > 0)
			{
				uint8_t *resp = NULL;
				uint32_t resp_len;

				rv = nfc_se_send_apdu_internal(seHandle,
					(uint8_t *)command.getBuffer(),
					command.size(),
					&resp,
					&resp_len);
				if (rv == NFC_ERROR_NONE &&
					resp != NULL)
				{
					response.assign(resp, resp_len);

					g_free(resp);
				}
				else
				{
					_ERR("nfc_se_send_apdu_internal failed, [%d]", rv);
 				}
			}
			else
			{
				_ERR("invalid command");
			}
		}
		else
		{
			_ERR("closed...");
		}

		_END();

		return rv;
	}

	int NFCTerminal::getATRSync(ByteArray &atr)
	{
		int rv = -1;

		_BEGIN();

		if (isClosed() == false)
		{
			uint8_t *temp = NULL;
			uint32_t temp_len;

			rv = nfc_se_get_atr_internal(seHandle, &temp, &temp_len);
			if (rv == NFC_ERROR_NONE && temp != NULL)
			{
				atr.assign(temp, temp_len);
				g_free(temp);
			}
			else
			{
				_ERR("nfc_se_get_atr_internal failed");
			}
		}
		else
		{
			_ERR("closed...");
		}

		_END();

		return rv;
	}
} /* namespace smartcard_service_api */
