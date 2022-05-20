#ifndef AUDIOCOMMONS_H
#define AUDIOCOMMONS_H
#include <mmdeviceapi.h>
#include <vector>

class AudioCommons
{
private:
    IMMDeviceEnumerator* pEnumerator = nullptr;
    LPWSTR pSelectedDeviceID = nullptr;
    std::vector<LPWSTR> activeEndpoints;

    void clearEndpointVector();
public:
    AudioCommons();
    ~AudioCommons();
    IMMDeviceEnumerator* getEnumerator(void) const;
    LPWSTR getSelectedDeviceID(void) const;
    void refreshEndpoints(void);
    const std::vector<LPWSTR>& getEndpoints(void) const;
};

#endif // AUDIOCOMMONS_H
