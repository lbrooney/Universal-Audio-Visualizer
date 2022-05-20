#ifndef AUDIOINTERFACE_H
#define AUDIOINTERFACE_H

#include <QWidget>

#include "Audio/audiocommons.h"
#include "Audio/audiorecorder.h"
#include "Audio/endpointnotificationclient.h"
#include <vector>

class AudioInterface
{
private:
    AudioCommons* pCommon = nullptr;
    AudioRecorder* pRecorder = nullptr;
    EndpointNotificationClient* pNotifier = nullptr;


public:
    AudioInterface();
    ~AudioInterface();
    AudioRecorder* getRecorder(void) const;
    const std::vector<LPWSTR> getEndpoints(void) const;
    IMMDeviceEnumerator* getEnumerator(void) const;
    const LPWSTR getSelectedDeviceID(void) const;

};

#endif // AUDIOINTERFACE_H
