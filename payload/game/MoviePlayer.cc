#include "MoviePlayer.hh"

MoviePlayer *MoviePlayer::Instance() {
    return s_instance;
}
