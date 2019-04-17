/*------------------------------------------------------------------------------
 Copyright © 2017 Continuum

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

  a. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  b. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  c. Neither the name of Continuum nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

Created by Jared Kirschner 2017
------------------------------------------------------------------------------*/

#include "include/gui_messages.h"

// 16 characters can fit per line. 4 lines max. \n marks end of line.

// System Statuses
const char * GUIMessages::StatusSystemInitializing = "SYSTEM\nINITIALIZING";
const char * GUIMessages::StatusInsertNewProbe = "INSERT NEW PROBE";
const char * GUIMessages::StatusCoreTemperatureOutOfRange = "CORE TEMPERATURE\nOUT OF RANGE";
const char * GUIMessages::StatusReadyToImageProbeSingleUse = "READY TO IMAGE";
// %02u:%02u should be filled with the HH:MM of probe life remaining
const char * GUIMessages::StatusReadyToImageProbeMultipleUse = "READY TO IMAGE\n\nIMAGING TIME\nREMAINING %02u:%02u";
const char * GUIMessages::StatusPreparingToImage = "PREPARING TO\nIMAGE";
const char * GUIMessages::StatusImaging = "IMAGING";
const char * GUIMessages::StatusImagingTemperatureThresholdExceeded = "IMAGING - \nTEMPERATURE\nTHRESHOLD\nEXCEEDED";
const char * GUIMessages::StatusImagingProbeWillExpireInNMinutes = "IMAGING\nPROBE WILL\nEXPIRE IN\n%u MINUTES";
const char * GUIMessages::StatusImagingProbeWillExpireIn1Minute = "IMAGING\nPROBE WILL\nEXPIRE IN\n1 MINUTE";
const char * GUIMessages::StatusStandBy = "STAND BY";
const char * GUIMessages::StatusServiceDue = "PIU SERVICE DUE";
const char * GUIMessages::StatusShuttingDown = "SHUTTING DOWN...";

// Error messages (pop-ups over System Statuses)
const char * GUIMessages::ErrorFaultDetectedInsertNewProbe = "FAULT DETECTED,\nINSERT NEW PROBE";
const char * GUIMessages::ErrorFaultDetectedRemoveProbeRebootSystem = "FAULT DETECTED,\nREMOVE PROBE,\nRE-BOOT SYSTEM";
const char * GUIMessages::ErrorFaultDetectedRebootSystem = "FAULT DETECTED,\nRE-BOOT SYSTEM";
const char * GUIMessages::ErrorProbeWillExpireSoon = "PROBE WILL\nEXPIRE SOON";
const char * GUIMessages::ErrorProbeExpiredInsertNewProbe = "PROBE EXPIRED\nINSERT NEW PROBE";
const char * GUIMessages::ErrorCoreTemperatureOutOfRange = GUIMessages::StatusCoreTemperatureOutOfRange;
