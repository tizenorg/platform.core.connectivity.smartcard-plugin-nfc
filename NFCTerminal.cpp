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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

/* SLP library header */
#include "net_nfc.h"
#include "net_nfc_internal_se.h"

/* local header */
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

/* below trhee functions must be implemented */
extern "C" EXPORT_API const char *get_name()
{
	return se_name;
}

extern "C" EXPORT_API void *create_instance()
{
	return (void *)NFCTerminal::getInstance();
}

extern "C" EXPORT_API void destroy_instance(void *instance)
{
	NFCTerminal *inst = (NFCTerminal *)instance;
	if (inst == NFCTerminal::getInstance())
	{
		inst->finalize();
	}
	else
	{
		SCARD_DEBUG_ERR("instance is invalid : getInstance [%p], instance [%p]", NFCTerminal::getInstance(), instance);
	}
}

namespace smartcard_service_api
{
	NFCTerminal::NFCTerminal():Terminal()
	{
		seHandle = NULL;
		closed = true;
		name = (char *)se_name;

		if (initialize())
		{
			/* TODO : disable nfc library temporary */
//			open();
		}
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
		int ret = 0;

		if (initialized == false)
		{
#if 0
			if ((ret = net_nfc_initialize()) == NET_NFC_OK)
			{
				if ((ret = net_nfc_set_response_callback(&NFCTerminal::nfcResponseCallback, this)) == NET_NFC_OK)
				{
					SCARD_DEBUG("nfc initialize success");

					initialized = true;
				}
				else
				{
					SCARD_DEBUG_ERR("net_nfc_set_response_callback failed [%d]", ret);
				}
			}
			else
			{
				SCARD_DEBUG_ERR("net_nfc_initialize failed [%d]", ret);
			}
#endif
		}

		return initialized;
	}

	void NFCTerminal::finalize()
	{
		if (isInitialized() && isClosed() == false && seHandle != NULL)
		{
			net_nfc_close_internal_secure_element(seHandle, this);
		}

		net_nfc_deinitialize();
	}

	bool NFCTerminal::open()
	{
		bool result = true;
		net_nfc_error_e ret;

		SCARD_BEGIN();

		if (isClosed() == true)
		{
#if 0
			if ((ret = net_nfc_open_internal_secure_element(NET_NFC_SE_TYPE_ESE, this)) == NET_NFC_OK)
			{
#ifndef ASYNC
				int rv;
				syncLock();
				if ((rv = waitTimedCondition(3)) == 0 && error == NET_NFC_OK)
				{
#endif
					SCARD_DEBUG("net_nfc_open_internal_secure_element returns [%d]", ret);
#ifndef ASYNC
				}
				else
				{
					SCARD_DEBUG_ERR("net_nfc_open_internal_secure_element failed cbResult [%d], rv [%d]", error, rv);
					result = false;
				}
				syncUnlock();
#endif
			}
			else
			{
				SCARD_DEBUG_ERR("net_nfc_set_secure_element_type failed [%d]", ret);
				result = false;
			}
#endif
		}

		SCARD_END();

		return result;
	}

	void NFCTerminal::close()
	{
		net_nfc_error_e ret;

		SCARD_BEGIN();

		if (isInitialized() && isClosed() == false && seHandle != NULL)
		{
			if ((ret = net_nfc_close_internal_secure_element(seHandle, this)) == NET_NFC_OK)
			{
#ifndef ASYNC
				int rv;

				syncLock();
				if ((rv = waitTimedCondition(3)) == 0 && error == NET_NFC_OK)
				{
#endif
					SCARD_DEBUG("net_nfc_close_internal_secure_element returns [%d]", ret);
#ifndef ASYNC
				}
				else
				{
					SCARD_DEBUG_ERR("net_nfc_close_internal_secure_element failed, error [%d], rv [%d]", error, rv);
				}
				syncUnlock();
#endif
			}
			else
			{
				SCARD_DEBUG_ERR("net_nfc_close_internal_secure_element failed [%d]", ret);
			}
		}

		SCARD_END();
	}

	bool NFCTerminal::isClosed()
	{
		return closed;
	}

	int NFCTerminal::transmitSync(ByteArray command, ByteArray &response)
	{
		int rv = 0;
		data_h data;

		SCARD_BEGIN();

		if (isClosed() == false)
		{
			SCOPE_LOCK(mutex)
			{
				if (command.getLength() > 0)
				{
					SCARD_DEBUG("command : %s", command.toString());

#ifndef ASYNC
					response.releaseBuffer();
#endif
					net_nfc_create_data(&data, command.getBuffer(), command.getLength());
					net_nfc_send_apdu(seHandle, data, this);
#ifndef ASYNC
					syncLock();
					rv = waitTimedCondition(3);

					if (rv == 0 && error == NET_NFC_OK)
					{
						SCARD_DEBUG("transmit success, length [%d]", response.getLength());
					}
					else
					{
						SCARD_DEBUG_ERR("transmit failed, rv [%d], cbResult [%d]", rv, error);
					}
					syncUnlock();

					rv = error;
#endif
					net_nfc_free_data(data);
				}
				else
				{
					rv = -1;
				}
			}
		}
		else
		{
			rv = -1;
		}

		SCARD_END();

		return rv;
	}

	int NFCTerminal::getATRSync(ByteArray &atr)
	{
		int rv = 0;

		SCARD_BEGIN();

		SCOPE_LOCK(mutex)
		{
			/* TODO : implement nfc first */
		}

		SCARD_END();

		return rv;
	}

	bool NFCTerminal::isSecureElementPresence()
	{
		return (seHandle != NULL);
	}

	void NFCTerminal::nfcResponseCallback(net_nfc_message_e message, net_nfc_error_e result, void *data , void *userContext, void *transData)
	{
		NFCTerminal *instance = (NFCTerminal *)userContext;

		SCARD_BEGIN();

		if (instance == NULL)
		{
			SCARD_DEBUG_ERR("instance is null");
			return;
		}

		switch(message)
		{
		case NET_NFC_MESSAGE_SET_SE :
			SCARD_DEBUG("NET_NFC_MESSAGE_SET_SE");
			break;

		case NET_NFC_MESSAGE_GET_SE :
			SCARD_DEBUG("NET_NFC_MESSAGE_GET_SE");
			break;

		case NET_NFC_MESSAGE_OPEN_INTERNAL_SE :
			SCARD_DEBUG("NET_NFC_MESSAGE_OPEN_INTERNAL_SE");

			if (result == NET_NFC_OK)
			{
				if (data != NULL)
				{
					instance->seHandle = (net_nfc_target_handle_h)data;
					instance->closed = false;
				}
				else
				{
					SCARD_DEBUG_ERR("NET_NFC_MESSAGE_OPEN_INTERNAL_SE failed");
				}
			}
			else
			{
				SCARD_DEBUG_ERR("NET_NFC_MESSAGE_OPEN_INTERNAL_SE returns error [%d]", result);
			}

			instance->error = result;

#ifndef ASYNC
			instance->syncLock();
			instance->signalCondition();
			instance->syncUnlock();
#else
			/* TODO : async process */
#endif
			break;

		case NET_NFC_MESSAGE_CLOSE_INTERNAL_SE :
			SCARD_DEBUG("NET_NFC_MESSAGE_CLOSE_INTERNAL_SE");

			if (result == NET_NFC_OK)
			{
				instance->closed = true;
			}
			else
			{
				SCARD_DEBUG_ERR("NET_NFC_MESSAGE_CLOSE_INTERNAL_SE failed [%d]", result);
			}

			instance->error = result;

#ifndef ASYNC
			instance->syncLock();
			instance->signalCondition();
			instance->syncUnlock();
#else
			/* TODO : async process */
#endif
			break;

		case NET_NFC_MESSAGE_SEND_APDU_SE :
			{
				data_h resp = (data_h)data;

				SCARD_DEBUG("NET_NFC_MESSAGE_SEND_APDU_SE");

				if (result == NET_NFC_OK)
				{
					if (resp != NULL)
					{
						SCARD_DEBUG("apdu result length [%d]", net_nfc_get_data_length(resp));
						instance->response.setBuffer(net_nfc_get_data_buffer(resp), net_nfc_get_data_length(resp));
					}
				}
				else
				{
					SCARD_DEBUG_ERR("NET_NFC_MESSAGE_OPEN_INTERNAL_SE failed [%d]", result);
				}

				instance->error = result;

#ifndef ASYNC
				instance->syncLock();
				instance->signalCondition();
				instance->syncUnlock();
#else
				/* TODO : async process */
#endif
			}
			break;

		case NET_NFC_MESSAGE_INIT :
			{
				instance->finalize();

				/* send notification */
				if (instance->statusCallback != NULL)
				{
					instance->statusCallback((void *)se_name, NOTIFY_SE_AVAILABLE, 0, NULL);
				}
			}
			break;

		case NET_NFC_MESSAGE_DEINIT :
			{
				instance->initialize();
				if (instance->open() == true)
				{
					/* send notification */
					if (instance->statusCallback != NULL)
					{
						instance->statusCallback((void *)se_name, NOTIFY_SE_NOT_AVAILABLE, 0, NULL);
					}
				}
			}
			break;

		default:
			SCARD_DEBUG("unknown message : [%d], [%d], [%p], [%p], [%p]", message, result, data, userContext, instance);
			break;
		}

		SCARD_END();
	}
} /* namespace smartcard_service_api */

