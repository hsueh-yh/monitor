/* ----------------------------------------------------------------------------
   libconfig - A library for processing structured configuration files
   Copyright (C) 2005-2010  Mark A Lindner

   This file is part of libconfig.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

#include "config.h"
#include <glogger.h>

using namespace std;
using namespace libconfig;

int readConfiger( std::string configFIle, CParams* pObj )
{
    Config cfg;

    //**************************************************
    // Read the file. If there is an error, report it and exit.
    try
    {
        cfg.readFile(configFIle.c_str());
    }
    catch(const FileIOException &fioex)
    {
        std::cerr << "I/O error while reading file." << std::endl;
        return(EXIT_FAILURE);
    }
    catch(const ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                  << " - " << pex.getError() << std::endl;
        return(EXIT_FAILURE);
    }

    //**************************************************
    // Get the store name.
    try
    {
        string name = cfg.lookup("name");
        VLOG(LOG_INFO) << setw(20) << setfill(' ') << std::right << "[LoadConfig]\t"
                       << "Store name: " << name << endl << endl;
    }
    catch(const SettingNotFoundException &nfex)
    {
        cerr << "No 'name' setting in configuration file." << endl;
    }



    const Setting& root = cfg.getRoot();

    //**************************************************
    // Output a list of all books in the inventory.
    try
    {
        const Setting &params = root["client"]["general"];
        int count = params.getLength();

        // read GeneralParams
        GeneralParams *p = &(pObj->generalParams_);
        for(int i = 0; i < count; ++i)
        {
            const Setting &param = params[i];

            // Only output the record if all of the expected fields are present.

            if(!(param.lookupValue("host", p->host_)
               && param.lookupValue("port", p->portNum_)
               && param.lookupValue("type", p->transType_)
               && param.lookupValue("logs", p->logFile_)))
            continue;
        }
    }
    catch(const SettingNotFoundException &nfex)
    {
        VLOG(LOG_ERROR) << "not found config: client-general";
    }


    //**************************************************
    // Output a list of all books in the inventory.
    try
    {
        const Setting &params = root["client"]["consumer"];
        int count = params.getLength();

        for(int i = 0; i < count; ++i)
        {
            const Setting &param = params[i];

            // Only output the record if all of the expected fields are present.
            ConsumerParams *p = &(pObj->consumerParams_);

            if(!(param.lookupValue("slotNum", p->bufferSlotsNum_)
               && param.lookupValue("slotSize", p->slotSize_)
               && param.lookupValue("interstLifeTm", p->interestLifetime_)
               && param.lookupValue("jittSizeMs", p->jitterSizeMs_)))
                continue;
        }
    }
    catch(const SettingNotFoundException &nfex)
    {
        VLOG(LOG_ERROR) << "not found config: client-consumer";
    }


    //**************************************************
    // Output a list of all books in the inventory.
    try
    {
        const Setting &params = root["client"]["stream"];
        int count = params.getLength();

        for(int i = 0; i < count; ++i)
        {
            const Setting &param = params[i];

            // Only output the record if all of the expected fields are present.
            MediaStreamParams *p = &(pObj->mediaStreamParams_);

            string type;
            if(!(param.lookupValue("type", type)
               && param.lookupValue("name", p->streamName_)))
                continue;
            if( type == "video" )
                p->type_ = MediaStreamParams::MediaStreamTypeVideo;
            if( type == "audio" )
                p->type_ = MediaStreamParams::MediaStreamTypeAudio;
        }
    }
    catch(const SettingNotFoundException &nfex)
    {
        VLOG(LOG_ERROR) << "not found config: client-stream";
    }


    return(EXIT_SUCCESS);
}

