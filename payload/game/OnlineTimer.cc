#include "OnlineTimer.hh"

#include "game/Kart2DCommon.hh"

#include <payload/crypto/CubeRandom.hh>
#include <portable/Array.hh>

void OnlineTimer::init(u32 duration) {
    m_duration = duration;
    m_frame = duration;
}

void OnlineTimer::draw(const J2DGraphContext *graphContext) {
    m_screen.draw(0.0f, 0.0f, graphContext);
}

void OnlineTimer::calc() {
    u32 milliseconds = m_frame * 1000 / 60;
    if (m_frame != 0 && m_frame != m_duration) {
        u32 nextMilliseconds = (m_frame + 1) * 1000 / 60;
        milliseconds += CubeRandom::Instance()->get(nextMilliseconds - milliseconds);
    }
    u32 seconds = milliseconds / 1000;
    milliseconds %= 1000;

    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    Array<J2DPicture *, 3> millisecondPictures;
    for (u32 i = 0; i < millisecondPictures.count(); i++) {
        millisecondPictures[i] = m_screen.search("Digit%u", 0 + i)->downcast<J2DPicture>();
    }
    kart2DCommon->changeNumberTexture(milliseconds, millisecondPictures.values(),
            millisecondPictures.count(), true, false);
    Array<J2DPicture *, 2> secondPictures;
    for (u32 i = 0; i < secondPictures.count(); i++) {
        secondPictures[i] = m_screen.search("Digit%u", 3 + i)->downcast<J2DPicture>();
    }
    kart2DCommon->changeNumberTexture(seconds, secondPictures.values(), secondPictures.count(),
            true, false);

    for (u32 i = 0; i < 5; i++) {
        m_screen.search("Digit%u", i)->setAlpha(m_alpha);
    }
    m_screen.search("Sep0")->setAlpha(m_alpha);

    if (m_frame > 0) {
        m_frame--;
    }
}

void OnlineTimer::setAlpha(u8 alpha) {
    m_alpha = alpha;
}

bool OnlineTimer::hasExpired() const {
    return m_frame == 0;
}

OnlineTimer *OnlineTimer::Create(JKRArchive *archive) {
    if (!s_instance) {
        s_instance = new OnlineTimer(archive);
    }
    return s_instance;
}

void OnlineTimer::Destroy() {
    delete s_instance;
    s_instance = nullptr;
}

OnlineTimer *OnlineTimer::Instance() {
    return s_instance;
}

OnlineTimer::OnlineTimer(JKRArchive *archive) : m_duration(0), m_frame(0), m_alpha(0) {
    m_screen.set("OnlineTimer.blo", 0x40000, archive);

    m_screen.search("Sep0")->downcast<J2DPicture>()->changeTexture("MarioFontColon2.bti", 0);
}

OnlineTimer::~OnlineTimer() {}

OnlineTimer *OnlineTimer::s_instance = nullptr;
