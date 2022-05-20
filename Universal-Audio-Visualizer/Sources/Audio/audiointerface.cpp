#include "Audio/audiointerface.h"

AudioInterface::AudioInterface()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    pCommon = new AudioCommons();
    pRecorder = new AudioRecorder(pCommon);
    pNotifier = new EndpointNotificationClient(pCommon);
    return;
}

AudioInterface::~AudioInterface()
{
    delete pNotifier;
    delete pRecorder;
    delete pCommon;
    CoUninitialize();
    return;
}

AudioRecorder *AudioInterface::getRecorder(void) const
{
    return pRecorder;
}

const std::vector<LPWSTR> AudioInterface::getEndpoints(void) const
{
    return pCommon->getEndpoints();
}

IMMDeviceEnumerator* AudioInterface::getEnumerator(void) const
{
    return pCommon->getEnumerator();
}

const LPWSTR AudioInterface::getSelectedDeviceID(void) const
{
    return pCommon->getSelectedDeviceID();
}
