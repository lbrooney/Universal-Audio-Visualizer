#include "Audio/audiointerface.h"

AudioInterface::AudioInterface()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    pCommon = new AudioCommons();
    pRecorder = new AudioRecorder(pCommon);
    pNotifier = new EndpointNotificationClient();
    return;
}

AudioInterface::~AudioInterface()
{

    delete pCommon;
    delete pRecorder;
    delete pNotifier;
    CoUninitialize();
    return;
}

AudioRecorder *AudioInterface::getRecorder(void) const
{
    return pRecorder;
}
