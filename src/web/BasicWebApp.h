//
// Created by Jost on 02/05/2024.
//

#ifndef TESTWEBAPP_H
#define TESTWEBAPP_H

#include <string>

#include "TrackData.h"

namespace TrackMapper::Web {
    class BasicWebApp {
    public:
        explicit BasicWebApp(const std::string &filePath);
        ~BasicWebApp();
        void Start(TrackData &trackData) const;
        void Stop() const;

    private:
        // opaque pointer to avoid linking against crow when including this header
        struct impl;
        std::unique_ptr<impl> pImpl;
    };
} // namespace TrackMapper::Web

#endif // TESTWEBAPP_H
