#ifndef AUDIOCOMMONS_H
#define AUDIOCOMMONS_H
#include <mmdeviceapi.h>
#include <vector>

class AudioCommons
{
private:
    IMMDeviceEnumerator* pEnumerator = nullptr;
    UINT selectIDSpot = 0;
    UINT defaultIDSpot = -1;
    std::vector<LPWSTR> activeEndpoints;

    void refreshEndpoints(void);
    void clearEndpointVector();
    void printDeviceName(const LPWSTR input) const;
public:
    AudioCommons();
    ~AudioCommons();
    IMMDeviceEnumerator* getEnumerator(void) const;
    void getSelectedDeviceID(LPWSTR& input);
    const std::vector<LPWSTR>& getEndpoints(void) const;
    void setAudioEndpoint(const UINT);
};

#endif // AUDIOCOMMONS_H
