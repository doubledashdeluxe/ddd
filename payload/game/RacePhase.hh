#pragma once

class RacePhase {
public:
    enum {
        CourseDemo = 1,
        NoCourseDemo = 2,
        Reset = 3,
        Restart = 4,
        Replay = 5,
        Title = 6,
        // ...
        Running = 17,
        // ...
    };

private:
    RacePhase();
};
