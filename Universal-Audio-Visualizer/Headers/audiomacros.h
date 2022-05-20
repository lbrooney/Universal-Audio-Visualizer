#ifndef AUDIOMACROS_H
#define AUDIOMACROS_H

#define EXIT_ON_ERROR(hres)  \
                  if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
                  if ((punk) != nullptr)  \
                    { (punk)->Release(); (punk) = nullptr; }

#endif // AUDIOMACROS_H
