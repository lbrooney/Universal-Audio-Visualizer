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
    std::cout << "audio interface delete start";
    delete pNotifier;
    std::cout << "| notifier deleted";
    delete pRecorder;
    delete pCommon;
    CoUninitialize();
    std::cout << " | audio interface delete end" << std::endl;
    return;
}

AudioRecorder *AudioInterface::getRecorder(void) const
{
    return pRecorder;
}
