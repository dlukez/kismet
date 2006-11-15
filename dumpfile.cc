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

#include "config.h"
#include "dumpfile.h"
#include "getopt.h"

Dumpfile::Dumpfile() {
	fprintf(stderr, "FATAL OOPS: Dumpfile() called with no global registry\n");
	exit(1);
}

Dumpfile::Dumpfile(GlobalRegistry *in_globalreg) {
	globalreg = in_globalreg;
	resume = 0;
	dumped_frames = 0;

	if (globalreg->packetchain == NULL) {
		fprintf(stderr, "FATAL OOPS:  Dumpfile() called before packetchain built\n");
		exit(1);
	}

	if (globalreg->kismet_config == NULL) {
		fprintf(stderr, "FATAL OOPS: Dumpfile() called before config built\n");
		exit(1);
	}

	export_filter = new FilterCore(globalreg);
	vector<string> filterlines = 
		globalreg->kismet_config->FetchOptVec("filter_export");
	for (unsigned int fl = 0; fl < filterlines.size(); fl++) {
		if (export_filter->AddFilterLine(filterlines[fl]) < 0) {
			_MSG("Failed to add filter_export config line from the Kismet config "
				 "file.", MSGFLAG_FATAL);
			globalreg->fatal_condition = 1;
			return;
		}
	}
}

void Dumpfile::Usage(char *name) {
	printf(" *** Dump/Logging Options ***\n");
	printf(" -T, --log-types              Override activated log types\n"
		   " -t, --log-title              Override default log title\n"
		   " -n, --no-logging             Disable logging entirely\n");
}

string Dumpfile::ProcessConfigOpt(string in_type) {
	string logtypes, logtemplate, logname;
	int option_idx = 0;
	string retfname;

	// longopts for the packetsourcetracker component
	static struct option logfile_long_options[] = {
		{ "log-types", required_argument, 0, 'T' },
		{ "log-title", required_argument, 0, 't' },
		{ "no-logging", no_argument, 0, 'n' },
		{ 0, 0, 0, 0 }
	};

	if ((logtemplate = globalreg->kismet_config->FetchOpt("logtemplate")) == "") {
		_MSG("No 'logtemplate' specified in the Kismet config file.", MSGFLAG_FATAL);
		globalreg->fatal_condition = 1;
		return "";
	}
	
	// Hack the extern getopt index
	optind = 0;

	while (1) {
		int r = getopt_long(globalreg->argc, globalreg->argv,
							"-T:t:n", 
							logfile_long_options, &option_idx);
		if (r < 0) break;
		switch (r) {
			case 'T':
				logtypes = string(optarg);
				break;
			case 't':
				logname = string(optarg);
				break;
			case 'n':
				return "";
				break;
		}
	}

	if (logname.length() == 0 &&
		(logname = globalreg->kismet_config->FetchOpt("logdefault")) == "") {
		_MSG("No 'logdefault' specified on the command line or config file",
			 MSGFLAG_FATAL);
		globalreg->fatal_condition = 1;
		return "";
	}

	if (logtypes.length() == 0 &&
		(logtypes = globalreg->kismet_config->FetchOpt("logtypes")) == "") {
		_MSG("No 'logtypes' specified on the command line or config file", 
			 MSGFLAG_FATAL);
		globalreg->fatal_condition = 1;
		return "";
	}

	vector<string> typevec = StrTokenize(StrLower(logtypes), ",");
	in_type = StrLower(in_type); // lower local copy

	int factive = 0;
	for (unsigned int x = 0; x < typevec.size(); x++) {
		if (typevec[x] == in_type) {
			factive = 1;
			break;
		}
	}

	if (factive == 0) {
		return "";
	}

	// _MSG("Log file type '" + in_type + "' activated.", MSGFLAG_INFO);

	retfname = ConfigFile::ExpandLogPath(logtemplate, logname, in_type, 0, 0);

	return retfname;
}

int Dumpfile::ProcessRuntimeResume(string in_type) {
	// No runstate?  Bail w/ -1 so we know to ignore the results
	if (globalreg->runstate_config == NULL)
		return -1;

	GroupConfigFile *rcf = globalreg->runstate_config;

	// Fetch all the root level entities, this will give us all the groups
	// so we can look for the dumpfile groups
	vector<GroupConfigFile::GroupEntity *> rent;
	rent = rcf->FetchEntityGroup(NULL);

	for (unsigned int x = 0; x < rent.size(); x++) {
		if (rent[x]->name != "dumpfile")
			continue;

		// Explode if we don't have a 'type' option in this dumpfile group
		if (rcf->FetchOpt("type", rent[x]) == "") {
			_MSG("Expected 'type' in runstate dumpfile group", MSGFLAG_FATAL);
			globalreg->fatal_condition = 1;
			return -1;
		}

		// Skip if we're not the right type
		if (rcf->FetchOpt("type", rent[x]) != in_type)
			continue;

		// Explode if we don't have a path
		if ((fname = rcf->FetchOpt("path", rent[x])) == "") {
			_MSG("Expected 'path' in runstate dumpfile group", MSGFLAG_FATAL);
			globalreg->fatal_condition = 1;
			return -1;
		}

		// Try to parse the number of dumped frames, explode if we can't
		// get it.
		if (sscanf(rcf->FetchOpt("numdumped", rent[x]).c_str(), "%d",
				   &dumped_frames) != 1) {
			_MSG("Expected 'numdumped' in runstate dumpfile group", MSGFLAG_FATAL);
			globalreg->fatal_condition = 1;
			return -1;
		}

		// if we've gotten this far, we have a valid record and we've assigned it,
		// so set that we're resuming and return out
		resume = 1;
		return 1;
	}

	return 0;
}
