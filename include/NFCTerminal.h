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

/* Tizen library header */
#include "nfc.h"

/* local header */
#include "Terminal.h"
#include "Lock.h"

namespace smartcard_service_api
{
	class NFCTerminal: public Terminal
	{
	private:
		PMutex mutex;
		nfc_se_h seHandle;
		bool opening;
		bool closed;

		NFCTerminal();
		~NFCTerminal();

		bool checkClosed();

	public:
		static NFCTerminal *getInstance();

		bool initialize();
		void finalize();

		bool open();
		bool isClosed() const;
		void close();

		bool isSecureElementPresence() const;

		int transmitSync(const ByteArray &command, ByteArray &response);
		int getATRSync(ByteArray &atr);

		int transmit(const ByteArray &command, terminalTransmitCallback callback, void *userParam) { return -1; };
		int getATR(terminalGetATRCallback callback, void *userParam) { return -1; }
	};
} /* namespace smartcard_service_api */
#endif /* NFCTERMINAL_H_ */
