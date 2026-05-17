#ifndef TV_CONTROLLER_H
#define TV_CONTROLLER_H

#include "Tuner.h"
#include "remoteKey.h"
#include <string>
#include <vector>

class TVController {
private:
  Tuner *tuner;
  std::string processingCH;
  std::vector<int> favorites;

  void setTunerCh();

public:
  explicit TVController(Tuner *tuner);

  std::vector<int> getFavorites() const;
  void pushButton(remoteKey key);
};

#endif // TV_CONTROLLER_H