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


#ifndef NFCTERMINAL_H_
#define NFCTERMINAL_H_

/* standard library header */

/* SLP library header */
#include "net_nfc_typedef.h"

/* local header */
#include "Terminal.h"
#include "Lock.h"

namespace smartcard_service_api
{
	class NFCTerminal: public Terminal
	{
	private:
		PMutex mutex;
		net_nfc_target_handle_h seHandle;
		bool closed;
		/* temporary data for sync function */
		ByteArray response;
		int error;

		NFCTerminal();
		~NFCTerminal();

		static void nfcResponseCallback(net_nfc_message_e message, net_nfc_error_e result, void *data , void *userContext, void *transData);

	public:

		static NFCTerminal *getInstance();

		bool initialize();
		void finalize();

		bool open();
		bool isClosed();
		void close();

		bool isSecureElementPresence();

		int transmitSync(ByteArray command, ByteArray &response);
		int getATRSync(ByteArray &atr);

		int transmit(ByteArray command, terminalTransmitCallback callback, void *userParam) { return -1; };
		int getATR(terminalGetATRCallback callback, void *userParam) { return -1; }

		friend void nfcResponseCallback(net_nfc_message_e message, net_nfc_error_e result, void *data , void *userContext, void *transData);
	};

} /* namespace smartcard_service_api */
#endif /* NFCTERMINAL_H_ */
