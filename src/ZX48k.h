#pragma once

#include "Speccy.h"
#include "Types.h"

namespace rm {
    class ZX48k : public Speccy {
    public:
        ZX48k();
        virtual ~ZX48k();

        void reset(bool coldBoot) override;

    protected:
    };
}
