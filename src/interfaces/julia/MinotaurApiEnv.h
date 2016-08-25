#ifndef MINOTAURAPIENV_H_
#define MINOTAURAPIENV_H_

#include "Types.h"

namespace minotaur_julia {
    class JuliaInterface;
    typedef JuliaInterface * JuliaInterfacePtr;
}
namespace Minotaur {

    class Environment;
    class Problem;

    class MinotaurApiEnv {
    public:
        MinotaurApiEnv();
        ~MinotaurApiEnv();

        Environment * env_;
        Problem *  problem_;
        minotaur_julia::JuliaInterfacePtr iface_;
    };

}

#endif /*MINOTAURAPIENV_H_ */
