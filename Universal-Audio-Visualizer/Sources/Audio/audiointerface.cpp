#include "Audio/audiointerface.h"
#include <QDebug>

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

void AudioInterface::getSelectedDeviceID(LPWSTR& input)
{
    pCommon->getSelectedDeviceID(input);
    return;
}

bool AudioInterface::setAudioEndpoint(const LPWSTR input)
{
#ifdef QT_DEBUG
    qDebug() << "input " << QString::fromWCharArray(input, -1) << Qt::endl;
#endif
    if(wcscmp(input, L"Default") == 0)
    {
        pCommon->setAudioEndpoint(-1);
        return true;
    }
    for(UINT i = 0; i < pCommon->getEndpoints().size(); i += 1)
    {
        if(wcscmp(input, pCommon->getEndpoints().at(i)) == 0)
        {
            pCommon->setAudioEndpoint(i);
            return true;
        }
    }
    return false;
}
