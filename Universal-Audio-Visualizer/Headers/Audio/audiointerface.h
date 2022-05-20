#ifndef AUDIOINTERFACE_H
#define AUDIOINTERFACE_H

#include <QWidget>

#include "Audio/audiocommons.h"
#include "Audio/audiorecorder.h"
#include "Audio/endpointnotificationclient.h"

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
};

#endif // AUDIOINTERFACE_H
