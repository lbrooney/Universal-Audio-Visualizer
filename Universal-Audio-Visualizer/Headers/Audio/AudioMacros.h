#ifndef AUDIOMACROS_H
#define AUDIOMACROS_H

#define EXIT_ON_ERROR(hres)  \
                  if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
                  if ((punk) != NULL)  \
                    { (punk)->Release(); (punk) = NULL; }

#endif // AUDIOMACROS_H
