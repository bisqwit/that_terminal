#include <tuple>
#include <utility>

struct JitModulo
{
    typedef unsigned (*ModuloFunc)(std::size_t);

    ModuloFunc MakeMod(unsigned val);
    ModuloFunc MakeDiv(unsigned val);
    ModuloFunc MakeMul(unsigned val);

    JitModulo() = default;
    JitModulo(JitModulo&& b) { *this = std::move(b); }
    JitModulo& operator=(JitModulo&& b)
    {
        if(this != &b)
        {
            std::tie(last,storage,used,allocated,prev_start) = std::tie(b.last,b.storage,b.used,b.allocated,b.prev_start);
            b.last       = {0,~0u};
            b.storage    = nullptr;
            b.used       = 0;
            b.allocated  = 0;
            b.prev_start = nullptr;
        }
        return *this;
    }

    JitModulo(const JitModulo&) = delete;
    JitModulo& operator=(const JitModulo&) = delete;

    ~JitModulo();

private:
    void Allocate();

public:
    std::pair<unsigned,unsigned> last{0,~0u};
    char* storage         = nullptr;
    std::size_t used      = 0;
    std::size_t allocated = 0;
    char* prev_start      = nullptr;
};
