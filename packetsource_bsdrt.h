/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
 * BSD is the generic BSD layer for capturing packets and controlling interfaces
 *
 */

#ifndef __PACKETSOURCE_BSD_H__
#define __PACKETSOURCE_BSD_H__

#include "config.h"

#if (defined(HAVE_LIBPCAP) && (defined(SYS_OPENBSD) || defined(SYS_NETBSD) || \
							   defined(SYS_FREEBSD)))

#include <string>
#include <errno.h>

#include "globalregistry.h"
#include "messagebus.h"

#include "packet.h"
#include "packet_ieee80211.h"
#include "packetsource.h"
#include "packetsource_pcap.h"

#define USE_PACKETSOURCE_BSDRT

// Prism 802.11 headers from the openbsd Hermes drivers, even though they don't return
// a valid linktype yet.  Structure lifted from bsd_airtools by dachb0den labs.
typedef struct {
	u_int16_t wi_status;
	u_int16_t wi_ts0;
	u_int16_t wi_ts1;
	u_int8_t  wi_silence;
	u_int8_t  wi_signal;
	u_int8_t  wi_rate;
	u_int8_t  wi_rx_flow;
	u_int16_t wi_rsvd0;
	u_int16_t wi_rsvd1;
} bsd_80211_header;

#define KDLT_BSD802_11			-100
#define KDLT_IEEE802_11_RADIO	127

// BSD packet source controller class, handles all the mode, channel, etc setting.
// Thanks to Sam Leffler and Pedro la Peu for the original variant and OpenBSD 
// updates of this
class Radiotap_BSD_Controller {
public:
	Radiotap_BSD_Controller(GlobalRegistry *in_globalreg, string in_dev);
	~Radiotap_BSD_Controller();

	int MonitorEnable(int initch);
	int MonitorReset(int initch);
	int ChangeChannel(int in_ch);

	int GetMediaOpt(int& options, int& mode);
	int SetMediaOpt(int options, int mode);
	int GetIfFlags(int &flags);
	int SetIfFlags(int value);
	int Get80211(int type, int& val, int len, uint8_t *data);
	int Set80211(int type, int val, int len, uint8_t *data);

protected:
	GlobalRegistry *globalreg;

	int CheckSocket();

	int sock;
	int prev_flags;
	int prev_options;
	int prev_mode;
	int prev_chan;

	string dev;
};

// BSD radiotap
class PacketSource_BSDRT : public PacketSource_Pcap {
public:
	// HANDLED PACKET SOURCES:
	// radiotap_bsd_ag
	// radiotap_bsd_a
	// radiotap_bsd_g
	// radiotap_bsd_b
	PacketSource_BSDRT() {
		fprintf(stderr, "FATAL OOPS:  Packetsource_BSDRT() called\n");
		exit(1);
	}

	PacketSource_BSDRT(GlobalRegistry *in_globalreg) :
		PacketSource_Pcap(in_globalreg) {
			bsdcon = NULL;
	}

	virtual KisPacketSource *CreateSource(GlobalRegistry *in_globalreg, 
										  string in_type, string in_name, 
										  string in_dev) {
		return new PacketSource_BSDRT(in_globalreg, in_type, in_name, in_dev);
	}

	virtual int AutotypeProbe(string in_device);
	virtual int RegisterSources(Packetsourcetracker *tracker);

	PacketSource_BSDRT(GlobalRegistry *in_globalreg, string in_type,
					   string in_name, string in_dev) :
		PacketSource_Pcap(in_globalreg, in_type, in_name, in_dev) {
			bsdcon = new Radiotap_BSD_Controller(in_globalreg, in_dev.c_str());
		}
	virtual ~PacketSource_BSDRT() { }

	virtual int OpenSource();

	virtual int FetchChannelCapable() { return 1; }

	virtual int EnableMonitor();
	virtual int DisableMonitor();
	virtual int SetChannel(unsigned int in_ch);
	virtual int SetChannelSequence(vector<unsigned int> in_seq);
	virtual int FetchChannel();
	
protected:
	Radiotap_BSD_Controller *bsdcon;

	// Override data link type to handle bsd funky bits
	virtual int DatalinkType();

	// BSD radio fetch
	virtual void FetchRadioData(kis_packet *in_packet);

	// Check that we support the dlt we need
	virtual int CheckDLT(int dlt);
};	

#endif /* have_libpcap && BSD */

#endif
