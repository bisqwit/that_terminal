#include <sys/mman.h>
#include <array>
#include <cstring>
#include <functional>
#include <bit>
#include <cassert>
#include <tuple>
#include <cstdio>
#include <cmath>

#include "jitmod.hh"

enum : unsigned { RAX = 0, RCX = 1, RDX = 2,  RBX = 3,  RSP = 4,  RBP = 5,  RSI = 6,  RDI = 7,
                  R8  = 8, R9  = 9, R10 = 10, R11 = 11, R12 = 12, R13 = 13, R14 = 14, R15 = 15,
                  ALL_REGS = 0xFFFF, // 16 bits
                  // Callee-saves registers:
                  BARRED = std::rotl(1u,RBX)
                         | std::rotl(1u,RBP)
                         | std::rotl(1u,RSP)
                         | std::rotl(1u,R12)
                         | std::rotl(1u,R13)
                         | std::rotl(1u,R14)
                         | std::rotl(1u,R15) };

struct Synther
{
    static constexpr unsigned Reg3x1 = 0xF0;
    static constexpr unsigned Reg3x2 = 0xE0;
    static constexpr unsigned Reg3x4 = 0xD0;
    static constexpr unsigned Reg3x8 = 0xC0;
    static constexpr unsigned Reg8x1 = 0xB0;
    static constexpr unsigned Reg8x2 = 0xA0;
    static constexpr unsigned Reg8x4 = 0x90;
    static constexpr unsigned Reg8x8 = 0x80;

    unsigned char* target;

public:
    void Synth(auto... byte)
    {
        std::array values{ ((unsigned char)byte)... };
        memcpy(target, &values[0], sizeof...(byte));
        target += sizeof...(byte);
    }
    void Synth32(unsigned val)
    {
        Synth(val&0xFF, (val>>8)&0xFF, (val>>16)&0xFF, (val>>24)&0xFF);
    }
    void Synth64(unsigned long long val)
    {
        Synth32(val); Synth32(val >> 32);
    }

    template<std::size_t A, std::size_t B>
    void SynthWith(std::array<unsigned,A> bytes, std::array<unsigned,B> regs)
    {
        // Annotations:
        // AABBCCnn
        for(auto& b: bytes)
        {
            //std::printf("0x%08X\n", b);
            if(b >= 256)
                for(unsigned n=8; n<=24; n+=8)
                {
                    //std::printf("\t%08X -- %u\n", (unsigned char)(b >> n), (unsigned char)(b >> n) >> 4);
                    switch(unsigned char c = (b >> n); c>>4)
                    {
                        case Reg3x1>>4: b |= (regs[c&0xF] & 7)*1; break;
                        case Reg3x2>>4: b |= (regs[c&0xF] & 7)*2; break;
                        case Reg3x4>>4: b |= (regs[c&0xF] & 7)*4; break;
                        case Reg3x8>>4: b |= (regs[c&0xF] & 7)*8; break;
                        case Reg8x1>>4: b |= 1*((regs[c&0xF]&8) >>3); break;
                        case Reg8x2>>4: b |= 2*((regs[c&0xF]&8) >>3); break;
                        case Reg8x4>>4: b |= 4*((regs[c&0xF]&8) >>3); break;
                        case Reg8x8>>4: b |= 8*((regs[c&0xF]&8) >>3); break;
                    }
                    //printf("\t->%08X\n", b);
                }
        }
        if(bytes[0] == 0x40)
        {
            // Skip 0x40 prefix
            for(std::size_t p=1; p<bytes.size(); ++p)
                target[p-1] = bytes[p];
            target += bytes.size()-1;
        }
        else
        {
            for(std::size_t p=0; p<bytes.size(); ++p)
                target[p] = bytes[p];
            target += bytes.size();
        }
    }
    template<typename... T>
    constexpr unsigned With(unsigned byte, T... params) const
    {
        std::array<unsigned, sizeof...(T)> p{ params... };
        unsigned shift = 0;
        for(unsigned c: p) { shift += 8; byte |= (c<<shift); }
        return byte;
    }
    void SynthSHL(unsigned target_reg, unsigned val)
    {
        if(val == 1)
            SynthADD(target_reg, target_reg);
//            { SynthWith(std::array{With(0x48, Reg8x1+0), 0xD1u,With(0xE0, Reg3x1+0)},
//                        std::array{target_reg}); }
        else if(val)
            { SynthWith(std::array{With(0x48, Reg8x1+0), 0xC1u,With(0xE0, Reg3x1+0)},
                        std::array{target_reg}); Synth(val); }
    }
    void SynthSHR(unsigned target_reg, unsigned val)
    {
        if(val == 1)
            { SynthWith(std::array{With(0x48, Reg8x1+0), 0xD1u,With(0xE8, Reg3x1+0)},
                        std::array{target_reg}); }
        else if(val)
            { SynthWith(std::array{With(0x48, Reg8x1+0), 0xC1u,With(0xE8, Reg3x1+0)},
                        std::array{target_reg}); Synth(val); }
    }
    void SynthNEG(unsigned target_reg)
    {
        SynthWith(std::array{With(0x48, Reg8x1+0), 0xF7u,With(0xD8, Reg3x1+0)},
                  std::array{target_reg});
    }
    void SynthXOR0(unsigned target_reg)
    {
        // 0x40: optional prefix
        SynthWith(std::array{With(0x40, Reg8x1+0, Reg8x4+0), 0x31u,With(0xC0, Reg3x1+0, Reg3x8+0)},
                  std::array{target_reg});
    }
    void SynthADD(unsigned target_reg, unsigned rhs_reg)
    {
        SynthWith(std::array{With(0x48, Reg8x1+0, Reg8x4+1), 0x01u,With(0xC0, Reg3x1+0, Reg3x8+1)},
                  std::array{target_reg,rhs_reg});
    }
    void SynthSUB(unsigned target_reg, unsigned rhs_reg)
    {
        if(target_reg == rhs_reg) { SynthXOR0(target_reg); return; }
        SynthWith(std::array{With(0x48, Reg8x1+0, Reg8x4+1), 0x29u,With(0xC0, Reg3x1+0, Reg3x8+1)},
                  std::array{target_reg,rhs_reg});
    }
    void SynthMOV(unsigned target_reg, unsigned rhs_reg)
    {
        if(target_reg == rhs_reg) return;
        SynthWith(std::array{With(0x48, Reg8x1+0, Reg8x4+1), 0x89u,With(0xC0, Reg3x1+0, Reg3x8+1)},
                  std::array{target_reg,rhs_reg});
    }
    void SynthMOVimm(unsigned target_reg, unsigned long val)
    {
        // skip prefix if possible
        if(val < (1ull << 32))
        {
            SynthWith(std::array{With(0x40, Reg8x1+0), With(0xB8, Reg3x1+0)},
                      std::array{target_reg});
            Synth32(val);
        }
        else
        {
            SynthWith(std::array{With(0x48, Reg8x1+0), With(0xB8, Reg3x1+0)},
                      std::array{target_reg});
            Synth64(val);
        }
    }
    void SynthLEA2(unsigned target_reg, unsigned lhs_reg, unsigned rhs_reg, unsigned rhs_mul)
    {
        if(rhs_mul == 1 && (target_reg == lhs_reg || target_reg == rhs_reg))
        {
            SynthSum(target_reg, lhs_reg, rhs_reg);
            return;
        }
        // lea a, [b+c*n]
        unsigned mul = std::countr_zero(rhs_mul)*0x40;
        //std::printf("Lea(%u,%u,%u,%u->%02X)\n", target_reg, lhs_reg, rhs_reg, rhs_mul, mul);
        SynthWith(std::array{With(0x48, Reg8x4+0, Reg8x1+1, Reg8x2+2), 0x8Du,With(0x04,Reg3x8+0), With(mul,Reg3x1+1,Reg3x8+2)},
                  std::array{target_reg,lhs_reg,rhs_reg});
    }
    void SynthLEA1(unsigned target_reg, unsigned rhs_reg, unsigned rhs_mul)
    {
        // lea a, [b*n]
        switch(rhs_mul)
        {
            case 1: SynthMOV(target_reg, rhs_reg); break;
            case 2: SynthLEA2(target_reg, rhs_reg, rhs_reg, 1); break; // rhs+rhs, no multiplication
            case 4: // 48 + 2*r8 + 4*t8,8D,04 + 8*t3, 0x85+r3, Z32
                    SynthWith(std::array{With(0x48, Reg8x4+0, Reg8x2+1), 0x8Du,With(0x04,Reg3x8+0), With(0x85,Reg3x8+1)},
                              std::array{target_reg, rhs_reg}); Synth32(0); break;
            case 8: SynthWith(std::array{With(0x48, Reg8x4+0, Reg8x2+1), 0x8Du,With(0x04,Reg3x8+0), With(0xC5,Reg3x8+1)},
                              std::array{target_reg, rhs_reg}); Synth32(0); break;
        }
    }
    void SynthIMUL(unsigned target_reg, unsigned lhs_reg)
    {
        SynthWith(std::array{With(0x48, Reg8x4+0, Reg8x4+1), 0x0Fu,0xAFu, With(0xC0, Reg3x8+0, Reg3x1+1)},
                  std::array{target_reg,lhs_reg});
    }
    void SynthIMULimm(unsigned target_reg, unsigned lhs_reg, unsigned val)
    {
        if(val < 128)
            SynthWith(std::array{With(0x48, Reg8x4+0, Reg8x4+1), 0x6Bu, With(0xC0, Reg3x8+0, Reg3x1+1), val},
                      std::array{target_reg,lhs_reg});
        else
        {
            SynthWith(std::array{With(0x48, Reg8x4+0, Reg8x4+1), 0x69u, With(0xC0, Reg3x8+0, Reg3x1+1)},
                      std::array{target_reg,lhs_reg});
            Synth32(val);
        }
    }
    void SynthMULX(unsigned target1_reg, unsigned target2_reg, unsigned rhs_reg)
    {
        unsigned char opcode[5] = {0xC4,0x42,0x83,0xF6,0xC0}; // 3-byte VEX
        //printf("Mulx(%u,%u,%u)\n", target1_reg,target2_reg,rhs_reg);
        opcode[1] |= ((~rhs_reg & 8)    *0x20/8)
                   | ((~target1_reg & 8)*0x80/8);
        opcode[2] |= (~target2_reg & 15) * 8;
        opcode[4] |= (rhs_reg & 7) | ((target1_reg & 7)*8);
        Synth(opcode[0],opcode[1],opcode[2],opcode[3],opcode[4]);
    }
    void SynthMUL(unsigned rhs_reg)
    {
        SynthWith(std::array{With(0x48, Reg8x1+0), 0xF7u,With(0xE0, Reg3x1+0)},
                  std::array{rhs_reg});
    }
    void SynthSum(unsigned target_reg, unsigned lhs_reg, unsigned rhs_reg)
    {
        if(target_reg == lhs_reg)
            SynthADD(target_reg, rhs_reg);
        else if(target_reg == rhs_reg)
            SynthADD(target_reg, lhs_reg);
        else
            SynthLEA2(target_reg, lhs_reg, rhs_reg, 1);
    }
    void SynthSub(unsigned target_reg, unsigned lhs_reg, unsigned rhs_reg, unsigned forbidden_regs)
    {
        if(target_reg == lhs_reg)
        {
            // a = a-b
            SynthSUB(target_reg, rhs_reg);
        }
        else if(forbidden_regs & (1u << lhs_reg))
        {
            if(target_reg == rhs_reg)
            {
                // b = b-a
                // b = -b
                SynthSUB(target_reg, lhs_reg);
                SynthNEG(target_reg);
            }
            else
            {
                // t = a
                // t = t-b
                SynthMOV(target_reg, lhs_reg);
                SynthSUB(target_reg, rhs_reg);
            }
        }
        else
        {
            // a = a-b
            // t = a
            SynthSUB(lhs_reg, rhs_reg);
            SynthMOV(target_reg, lhs_reg);
        }
    }
    void SynthDivide(unsigned val, unsigned lhs_reg, unsigned target_reg, unsigned forbidden_regs)
    {
        if(!val) [[unlikely]]
        {
            SynthXOR0(target_reg);
            return;
        }
        if(std::has_single_bit(val))
        {
            SynthMOV(target_reg, lhs_reg);
            SynthSHR(target_reg, std::countr_zero(val));
        }
        else
        {
            // CHOOSE_MULTIPLIER: Figure 6.2 in https://gmplib.org/~tege/divcnst-pldi94.pdf
            constexpr unsigned N = 64;
            auto CHOOSE_MULTIPLIER = [](unsigned d, int prec)
            {
                // N    = Number of bits in divisor
                // prec = Number of bits of precision needed
                if(prec > int(N)) prec = int(N);

                //int l = std::bit_width(d) - !std::has_single_bit(d); // Least integer not less than log2(d)
                int l = std::ceil(log2(d));
                int shpost = l;
                unsigned pow1 = N+l;
                unsigned pow2 = N+l-prec;
                __uint128_t exp1 = __uint128_t(1) << pow1;
                __uint128_t exp2 = __uint128_t(1) << pow2;
                __uint128_t mlow  = exp1;           mlow /= d;
                __uint128_t mhigh = exp1 | exp2;   mhigh /= d;
                //printf("mlow=%llX mhigh=%llX shpost=%d\n", (unsigned long long)mlow, (unsigned long long)mhigh, shpost);
                while(mlow/2 < mhigh/2 && shpost > 0)
                {
                    mlow /= 2;
                    mhigh /= 2;
                    --shpost;
                    //printf("mlow=%llX mhigh=%llX shpost=%d\n", (unsigned long long)mlow, (unsigned long long)mhigh, shpost);
                }
                // Outputs: (mhigh, shpost, l)
                return std::tuple{mhigh, shpost, l};
            };
            auto [m, shpost, l] = CHOOSE_MULTIPLIER(val, 64);
            //printf("Chose m=%llX shpost=%d\n", (unsigned long long)m, shpost);

            // Figure 4.2 in https://gmplib.org/~tege/divcnst-pldi94.pdf
            int shpre = 0;
            if(m >= (__uint128_t(1)<<N) && val%2 == 0)
            {
                int ldummy;
                shpre = std::countr_zero(val);
                val >>= shpre;
                std::tie(m, shpost, ldummy) = CHOOSE_MULTIPLIER(val, N-shpre);
            }
            if(val == (1u << l))
            {
                SynthMOV(target_reg, lhs_reg);
                SynthSHR(target_reg, l);
            }
            else if(m >= (__uint128_t(1)<<N))
            {
                unsigned not_these = forbidden_regs | (1u << lhs_reg) | (1u << target_reg);
                assert(not_these != ALL_REGS);
                unsigned temporary = std::countr_one(not_these); // Find first available register
                /*
                    x1 = mulhi(const, LHS)  LIVE: LHS,x1
                    x2 = sub(LHS, x1)       LIVE: LHS,x1,x2
                    x3 = shr(x2, 1)         LIVE:     x1,   x3
                    x4 = add(x1, x3)        LIVE:           x3,x4
                    x5 = shr(x4, shpost-1)  LIVE:                x5

                    If LHS is modifiable:
                    temp   = mulhi(const, LHS)
                    LHS    = sub(LHS, temp)
                    LHS    = shr(x2, 1)
                    TARGET = LHS + temp
                    TARGET = shr(TARGET, shpost-1)

                    Otherwise:
                    TARGET = mulhi(const, LHS)
                    temp   = sub(LHS, TARGET)
                    temp   = shr(x2, 1)
                    TARGET = temp + TARGET
                    TARGET = shr(TARGET, shpost-1)
                */
                unsigned wip1 = temporary;
                unsigned wip2 = lhs_reg;
                if(forbidden_regs & (1u << lhs_reg))
                {
                    wip1 = target_reg;
                    wip2 = temporary;
                }
                //printf("target=%u lhs=%u now temp=%u\n", target_reg, lhs_reg, temporary);
                SynthMULHI(m - (__uint128_t(1)<<N), lhs_reg, wip1, forbidden_regs | (1u << lhs_reg));
                SynthSub(wip2, lhs_reg, wip1, forbidden_regs | (1u << wip1));
                SynthSHR(wip2, 1);
                SynthSum(target_reg, wip2, wip1);
                SynthSHR(target_reg, shpost-1);
            }
            else
            {
                if(forbidden_regs & (1u << lhs_reg))
                {
                    SynthMOV(target_reg, lhs_reg);
                    lhs_reg = target_reg;
                }
                SynthSHR(lhs_reg, shpre);
                SynthMULHI(m, lhs_reg, target_reg, forbidden_regs);
                SynthSHR(target_reg, shpost);
            }
        }
    }
    void SynthMULHI(unsigned long long val, unsigned lhs_reg, unsigned target_reg, unsigned forbidden_regs)
    {
        assert(!(forbidden_regs & (1u << RDX)));
        /* Options:
         * Use MULX.
         *   We need RDX as temp. It can also be target_reg.
         *   MULX(A,B,C) does this:
         *         a = rdx*c
         *         b = rdx*c >> 64
         * Or use MUL.
         *   We need RAX and RDX.
         */
        if(true) // Use MULX
        {
            if(lhs_reg == RDX)
            {
                SynthMOV(target_reg, lhs_reg);
                lhs_reg = target_reg;
            }
            SynthMOVimm(RDX, val);
            // Generate MULX target_reg, target_reg, lhs_reg
            SynthMULX(target_reg, target_reg, lhs_reg);
        }
        else // Use MUL
        {
            assert(!(forbidden_regs & (1u << RAX)));
            if(lhs_reg == RAX)
            {
                SynthMOVimm(RDX, val);
                SynthMUL(RDX);
            }
            else
            {
                SynthMOVimm(RAX, val);
                SynthMUL(lhs_reg);
            }
            SynthMOV(target_reg, RDX);
        }
    }
    void SynthMultiply(unsigned val, unsigned lhs_reg, unsigned target_reg, unsigned forbidden_regs)
    {
        //Synth(0x0F,0x1F,0x80); Synth32(val);
        //printf("SynthMultiply(%u): lhs=%u, target=%u, forbidden=0x%X\n", val, lhs_reg, target_reg, forbidden_regs);
        if(!val) [[unlikely]]
        {
            SynthXOR0(target_reg);
            return;
        }
        if(val == 1 && target_reg == lhs_reg)
        {
            return;
        }
        assert(! (forbidden_regs & (1u << target_reg)) );
        auto MakeLHSwritable = [&]()
        {
            if(forbidden_regs & (1u << lhs_reg))
            {
                SynthMOV(target_reg, lhs_reg);
                lhs_reg = target_reg;
            }
        };
        while(val != 1)
        {
            switch(val)
            {
                case 2: SynthSum(target_reg, lhs_reg, lhs_reg); lhs_reg = target_reg; val = 1; continue;
                case 4: SynthLEA1(target_reg, lhs_reg, 4); lhs_reg = target_reg; val = 1; continue;
                case 8: SynthLEA1(target_reg, lhs_reg, 8); lhs_reg = target_reg; val = 1; continue;
                case 3: SynthLEA2(target_reg, lhs_reg, lhs_reg, 2); lhs_reg = target_reg; val = 1; continue;
                case 5: SynthLEA2(target_reg, lhs_reg, lhs_reg, 4); lhs_reg = target_reg; val = 1; continue;
                case 9: SynthLEA2(target_reg, lhs_reg, lhs_reg, 8); lhs_reg = target_reg; val = 1; continue;
            }
            if(unsigned pow = std::countr_zero(val); pow > 0)
            {
                MakeLHSwritable();
                SynthSHL(lhs_reg, pow);
                val >>= pow;
                continue;
            }
            if(!(val % 9))
            {
                MakeLHSwritable();
                SynthLEA2(lhs_reg, lhs_reg,lhs_reg, 8);
                val /= 9;
                continue;
            }
            if(!(val % 5))
            {
                MakeLHSwritable();
                SynthLEA2(lhs_reg, lhs_reg,lhs_reg, 4);
                val /= 5;
                continue;
            }
            if(!(val % 3))
            {
                MakeLHSwritable();
                SynthLEA2(lhs_reg, lhs_reg,lhs_reg, 2);
                val /= 3;
                continue;
            }
            // Check if val is a composite of two nice numbers
            static const unsigned opts[] = {1u<<1, 3, 1u<<2, 5, 1u<<3, 9, 1u<<4,1u<<5,1u<<6,1u<<7,1u<<8,1u<<9,1u<<10,
            1u<<11,1u<<12,1u<<13,1u<<14,1u<<15,1u<<16,1u<<17,1u<<18,1u<<19,
            1u<<21,1u<<22,1u<<23,1u<<24,1u<<25,1u<<26,1u<<27,1u<<28,1u<<29,
            1u<<30,1u<<31,
            };
            std::tuple<unsigned,unsigned,int> operation{0,0,0}; unsigned op_score=0;
            auto score = [&](unsigned part1,unsigned part2)
            {
                unsigned result = 1;
                if(/*std::has_single_bit(part1) || */part1==1||part1==3||part1==5||part1==9) result += (1 + part1==1);
                if(/*std::has_single_bit(part2) || */part2==1||part2==3||part2==5||part2==9) result += (1 + part2==1);
                return result;
            };
            // TODO: How is 16+3 better than 9*2+1?
            for(auto o: opts)
            {
                unsigned part1 = o;
                if(part1 >= val) continue;
                unsigned part2 = val-part1;
                if(part2 == 1 || std::binary_search(std::begin(opts), std::end(opts), part2))
                    if(unsigned s = score(part1,part2); s > op_score) // part1+part2
                        std::tie(operation, op_score) = std::tuple{std::tuple{part1,part2,1}, s};
            }
            for(auto o: opts)
            {
                unsigned part1 = o;
                if(part1 <= val) continue;
                unsigned part2 = part1-val;
                if(part2 == 1 || std::binary_search(std::begin(opts), std::end(opts), part2))
                    if(unsigned s = score(part1,part2); s > op_score) // part1-part2
                        std::tie(operation, op_score) = std::tuple{std::tuple{part1,part2,-1}, s};
            }
            for(unsigned mod=2; mod<=8 && mod<val; mod*=2)
            {
                unsigned part1 = val / mod;  // 19/2 = 9
                unsigned part2 = val % mod;  // 19%2 = 1
                if(part1 == 1 || std::binary_search(std::begin(opts), std::end(opts), part1))
                if(part2 == 1 || std::binary_search(std::begin(opts), std::end(opts), part2))
                    if(unsigned s = score(part1,part2)+1; s > op_score) // part1*mod + part2
                        std::tie(operation, op_score) = std::tuple{std::tuple{part1,part2,mod}, s};
            }

            unsigned not_these = forbidden_regs | (1u << lhs_reg);
            switch(auto [part1,part2,pol] = operation; pol)
            {
                case 1: case 2: case 4: case 8:
                {
                    // Ok! Generate as lhs*part1 + lhs*part2*N;
                    assert(not_these != ALL_REGS);
                    unsigned temporary = std::countr_one(not_these); // Find first available register
                    if(part2*pol != 1 && part2*pol != 2 && part2*pol != 4 && part2*pol != 8)
                    {
                        temporary = std::countr_one(not_these | (1u << target_reg)); // Find first available register
                    }

                    SynthMultiply(part1, lhs_reg, temporary, not_these);       // Can use target_reg as temp, if != lhs_reg
                    //printf("Generating x[%u]*%u [%u] = x*%u (in %u) + x*%u*%u\n", lhs_reg,val,target_reg, part1,temporary, part2,pol);
                    switch(part2*pol)
                    {
                        case 1: case 2: case 4: case 8: SynthLEA2(target_reg, lhs_reg, temporary, part2*pol); break;
                        default:
                            unsigned dest = target_reg;
                            if(!(forbidden_regs & (1u << lhs_reg))) dest = lhs_reg;
                            SynthMultiply(part2, lhs_reg, dest, forbidden_regs | (1u << temporary)); // Can use lhs_reg as temp
                            SynthLEA2(target_reg, dest, temporary, pol);
                    }
                    return;
                }
                case -1:
                {
                    // Ok! Generate as lhs*part1 - lhs*part2;
                    assert(not_these != ALL_REGS);
                    unsigned temporary = std::countr_one(not_these | (1u << target_reg)); // Find first available register

                    //printf("Generating x[%u]*%u [%u] = x*%u (in %u) - x*%u\n", lhs_reg,val,target_reg, part1,temporary, part2);
                    unsigned dest1 = temporary, dest2 = target_reg;
                    unsigned src1  = lhs_reg,   src2  = lhs_reg;
                    if(part2 == 1 && lhs_reg != target_reg)
                    {
                        dest1 = target_reg;
                        dest2 = lhs_reg;
                    }
                    else if(dest1 != target_reg && !(forbidden_regs & (1u << lhs_reg)))
                    {
                        SynthMOV(temporary, lhs_reg);
                        dest1 = target_reg;
                        src2  = temporary;
                        dest2 = temporary;
                        not_these &= ~(1u << lhs_reg);
                        // temp   = lhs
                        // target = lhs*part1
                        // temp   = temp*part2
                        // target = lhs - target
                    }
                    SynthMultiply(part1, src1, dest1, not_these);                      // Can use target_reg as temp, if != lhs_reg
                    SynthMultiply(part2, src2, dest2, forbidden_regs | (1u << dest1)); // Can use lhs_reg as temp
                    SynthSub(target_reg, dest1, dest2, forbidden_regs);
                    return;
                }
            }
            // Too complex. Create imul.
            if(std::bit_width(val) <= 31)
            {
                // imul target_reg, lhs_reg, val
                SynthIMULimm(target_reg, lhs_reg, val);
            }
            else
            {
                // mov target_reg, val
                SynthMOVimm(target_reg, val);
                // imul target_reg, lhs_reg
                SynthIMUL(target_reg, lhs_reg);
            }
            return;
        }
        SynthMOV(target_reg, lhs_reg);
        //std::printf("Done mul %u\n", val);
    }
    void Fill(unsigned char* begin, unsigned char* cap)
    {
        std::size_t align = (target-begin) & 15;
        while(target < cap)
        {
            std::size_t nop_length = std::min<std::size_t>(16-align, cap-target);
            while(nop_length > 8) { Synth(0x66); --nop_length; }
            switch(nop_length)
            {
                case 1: Synth(0x90); break;
                case 2: Synth(0x40,0x90); break;
                case 3: Synth(0x0F,0x1F,0x00); break;
                case 4: Synth(0x0F,0x1F,0x40,0x00); break;
                case 5: Synth(0x0F,0x1F,0x44,0x00,0x00); break;
                case 6: Synth(0x66,0x0F,0x1F,0x44,0x00,0x00); break;
                case 7: Synth(0x0F,0x1F,0x80,0x00,0x00,0x00,0x00); break;
                case 8: Synth(0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00); break;
            }
            align = 0;
        }
    }
    unsigned Align(unsigned char* begin)
    {
        std::size_t align = (target-begin) & 15;
        if(align)
        {
            std::size_t nop_length = 16-align;
            while(nop_length > 8) { Synth(0x66); --nop_length; }
            switch(nop_length)
            {
                case 1: Synth(0x90); break;
                case 2: Synth(0x40,0x90); break;
                case 3: Synth(0x0F,0x1F,0x00); break;
                case 4: Synth(0x0F,0x1F,0x40,0x00); break;
                case 5: Synth(0x0F,0x1F,0x44,0x00,0x00); break;
                case 6: Synth(0x66,0x0F,0x1F,0x44,0x00,0x00); break;
                case 7: Synth(0x0F,0x1F,0x80,0x00,0x00,0x00,0x00); break;
                case 8: Synth(0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00); break;
            }
        }
        return target-begin;
    }
};

JitModulo::ModuloFunc JitModulo::MakeMod(unsigned mod)
{
    Allocate();
    if(last == std::pair(mod,0u)) [[unlikely]] return (ModuloFunc)prev_start;
    last = std::pair(mod,0u);

    prev_start = storage + used;
    Synther synth{ (unsigned char*) prev_start };

    #define MOV_RAX_RDI     0x48,0x89,0xF8
    #define XOR_EAX_EAX     0x31,0xC0
    #define RET             0xC3
    #define AND_EAX_IMM32   0x25
    #define AND_EAX_IMM7    0x83,0xE0

    /* Input: rdi, output: eax */

    /* If mod is a power of two, create an AND expression */
    if(mod == 1)
    {
        [[unlikely]]
        synth.Synth(XOR_EAX_EAX,
              RET);
    }
    else if(!(mod & (mod-1)))
    {
        if(mod <= 128)
        {
            synth.Synth(MOV_RAX_RDI,
                        AND_EAX_IMM7, mod-1,
                        RET);
        }
        else
        {
            synth.Synth(MOV_RAX_RDI,
                        AND_EAX_IMM32); synth.Synth32(mod-1);
            synth.Synth(RET);
        }
    }
    else
    {
        /* X % Y -> X - (X/Y) * Y */
        // rax = rdi / Y
        // rax = rax * Y
        // rax = rax - rdi
        synth.SynthDivide(mod,     RDI/*input*/, RAX/*target*/, BARRED | (1u << RDI)/*may not modify*/);
        synth.SynthMultiply(mod,   RAX/*input*/, RAX/*target*/, BARRED | (1u << RDI)/*may not modify*/);
        synth.SynthSub(RAX, RDI, RAX, BARRED);
        synth.Synth(RET);
    }
    used = synth.Align( (unsigned char*) storage );
    //synth.Fill( (unsigned char*) storage, (unsigned char*)(storage+4096) );
    return (ModuloFunc)prev_start;
}

JitModulo::ModuloFunc JitModulo::MakeDiv(unsigned mod)
{
    Allocate();
    if(last == std::pair(mod,1u)) [[unlikely]] return (ModuloFunc)prev_start;
    last = std::pair(mod,1u);

    prev_start = storage + used;
    Synther synth{ (unsigned char*) prev_start };

    /* Input: rdi, output: eax */
    synth.SynthDivide(mod, RDI/*input*/, RAX/*target*/, BARRED/*may not modify*/);
    synth.Synth(RET);

    //synth.Fill( (unsigned char*) storage, (unsigned char*)(storage+4096) );
    used = synth.Align( (unsigned char*) storage );
    return (ModuloFunc)prev_start;
}

JitModulo::ModuloFunc JitModulo::MakeMul(unsigned mod)
{
    Allocate();
    if(last == std::pair(mod,1u)) [[unlikely]] return (ModuloFunc)prev_start;
    last = std::pair(mod,1u);

    prev_start = storage + used;
    Synther synth{ (unsigned char*) prev_start };

    #define MOV_RAX_RDI     0x48,0x89,0xF8
    #define XOR_EAX_EAX     0x31,0xC0
    #define RET             0xC3
    #define AND_EAX_IMM32   0x25
    #define AND_EAX_IMM7    0x83,0xE0

    /* Input: rdi, output: eax */
    synth.SynthMultiply(mod, RDI/*input*/, RAX/*target*/, BARRED/*may not modify*/);
    synth.Synth(RET);

    //synth.Fill( (unsigned char*) storage, (unsigned char*)(storage+4096) );
    used = synth.Align( (unsigned char*) storage );
    return (ModuloFunc)prev_start;
}

JitModulo::~JitModulo()
{
    if(storage && (long long)storage != -1)
         munmap(storage, allocated);
}

void JitModulo::Allocate()
{
    if(!storage) [[unlikely]]
    {
        storage = (char*)mmap(nullptr, allocated = 1048576,
                              PROT_EXEC|PROT_READ|PROT_WRITE,
                              MAP_PRIVATE|MAP_ANONYMOUS, -1,0);
        used = 0;
    }
}
