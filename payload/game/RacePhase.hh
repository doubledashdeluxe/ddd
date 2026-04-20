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
        Safety = 16,
        Running = 17,
        Wait = 18,
        Ending = 19,
        // ...
    };

private:
    RacePhase();
};
