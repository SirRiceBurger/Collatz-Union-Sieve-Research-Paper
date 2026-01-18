#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <functional>
#include <limits>
#include <ctime>

using u64  = uint64_t;
using u32  = uint32_t;
using u128 = __uint128_t;

static inline std::string get_ts() {
    using namespace std::chrono;
    auto t = system_clock::to_time_t(system_clock::now());
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
    return std::string(buf);
}

static inline u64 T_step(u64 n, u64 &o_cnt) {
    if (n & 1ULL) {
        o_cnt++;
        u128 t = (u128)3 * (u128)n + 1;
        return (u64)(t >> 1);
    }
    return n >> 1;
}

static inline std::pair<u64,u64> get_Tk(u64 n, u32 k) {
    u64 o = 0;
    for (u32 i = 0; i < k; i++) n = T_step(n, o);
    return {n, o};
}

static inline bool p3_check(u64 e, u128 &res) {
    u128 v = 1;
    for (u64 i = 0; i < e; i++) {
        if (v > (std::numeric_limits<u128>::max() / 3)) return false;
        v *= 3;
    }
    res = v;
    return true;
}

static std::vector<uint8_t> init_sieve(u32 k) {
    u64 sz = 1ULL << k;
    std::vector<uint8_t> bset(sz, 0);
    u128 p2k = (u128)1 << k;

    for (u64 i = 0; i < sz; i++) {
        auto [tk, o] = get_Tk(i, k);
        u128 p3;
        if (!p3_check(o, p3)) continue;
        if (p2k <= p3) continue;
        if (p3 + (u128)tk < p2k + (u128)i) bset[i] = 1;
    }
    return bset;
}

static inline u32 run_fallback(u64 start, u32 limit) {
    u64 x = start;
    for (u32 i = 1; i <= limit; i++) {
        u64 o = 0;
        x = T_step(x, o);
        if (x < start) return i;
    }
    return limit;
}

struct run_stats {
    u64 n_total = 0;
    u64 n_pruned = 0;
    u64 n_sim = 0;
    u64 f_steps = 0;
};

struct bench_raw {
    run_stats s;
    double t_sec = 0.0;
};

static bench_raw get_median(std::function<bench_raw()> fn, u32 n) {
    std::vector<double> results;
    bench_raw best;
    for (u32 i = 0; i < n; i++) {
        bench_raw r = fn();
        if (i == 0) best = r;
        results.push_back(r.t_sec);
    }
    std::sort(results.begin(), results.end());
    best.t_sec = results[results.size() / 2];
    return best;
}

static std::vector<u32> parse_ks(const std::string &raw) {
    std::vector<u32> out;
    std::string tmp;
    std::stringstream ss(raw);
    while(std::getline(ss, tmp, ',')) {
        if(!tmp.empty()) out.push_back((u32)std::stoul(tmp));
    }
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

static bench_raw do_bench(u32 k, u64 nH, u64 amt, const std::vector<uint8_t> &tbl, u32 f_lim, bool check) {
    u64 mod = 1ULL << k;
    u64 start = nH * mod;
    bench_raw res;
    res.s.n_total = amt;

    auto t0 = std::chrono::high_resolution_clock::now();
    for (u64 i = 0; i < amt; i++) {
        u64 curr = start + i;
        if (tbl[curr & (mod - 1)]) {
            res.s.n_pruned++;
            if (check && (i % 200000 == 0)) {
                if (get_Tk(curr, k).first >= curr) exit(1);
            }
        } else {
            res.s.n_sim++;
            res.s.f_steps += run_fallback(curr, f_lim);
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    res.t_sec = std::chrono::duration<double>(t1 - t0).count();
    return res;
}

static bench_raw do_adaptive(const std::vector<u32> &ks, u64 nH, u64 amt, const std::vector<std::vector<uint8_t>> &tbls, u32 k_max, u32 f_lim, bool check) {
    u64 start = nH * (1ULL << k_max);
    bench_raw res;
    res.s.n_total = amt;

    auto t0 = std::chrono::high_resolution_clock::now();
    for (u64 i = 0; i < amt; i++) {
        u64 curr = start + i;
        bool p = false;
        u32 h_k = 0;
        for (size_t j = 0; j < ks.size(); j++) {
            if (tbls[j][curr & ((1ULL << ks[j]) - 1)]) {
                p = true; h_k = ks[j]; break;
            }
        }
        if (p) {
            res.s.n_pruned++;
            if (check && (i % 200000 == 0)) {
                if (get_Tk(curr, h_k).first >= curr) exit(1);
            }
        } else {
            res.s.n_sim++;
            res.s.f_steps += run_fallback(curr, f_lim);
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    res.t_sec = std::chrono::duration<double>(t1 - t0).count();
    return res;
}

int main(int argc, char **argv) {
    std::string ks_in = (argc > 1) ? argv[1] : "12,14,16,18,20";
    u64 d_amt = (argc > 2) ? std::stoull(argv[2]) : 200000;
    u64 a_amt = (argc > 3) ? std::stoull(argv[3]) : 800000;
    u32 reps = (argc > 4) ? (u32)std::stoul(argv[4]) : 3;
    u32 f_max = (argc > 5) ? (u32)std::stoul(argv[5]) : 20000;
    std::string out_f = (argc > 6) ? argv[6] : "results.csv";
    
    auto ks = parse_ks(ks_in);
    u32 k_max = ks.back();

    std::vector<std::vector<uint8_t>> s_list;
    for (u32 k : ks) s_list.push_back(init_sieve(k));

    std::vector<uint8_t> u_sieve(1ULL << k_max, 0);
    for (u64 i = 0; i < (1ULL << k_max); i++) {
        for (size_t j = 0; j < ks.size(); j++) {
            if (s_list[j][i & ((1ULL << ks[j]) - 1)]) { u_sieve[i] = 1; break; }
        }
    }

    std::ofstream f(out_f);
    f << "ts,type,nh,prune_base,prune_adapt,prune_uni,t_base,t_adapt,t_uni\n";

    auto sweep = [&](std::string label, std::vector<u64> nhs, u64 count) {
        std::cout << "Sweep: " << label << "\n";
        for (u64 nh : nhs) {
            auto r_base = get_median([&](){ return do_bench(k_max, nh, count, s_list.back(), f_max, (label=="dense"&&nh<=64)); }, reps);
            auto r_adapt = get_median([&](){ return do_adaptive(ks, nh, count, s_list, k_max, f_max, (label=="dense"&&nh<=64)); }, reps);
            auto r_uni = get_median([&](){ return do_bench(k_max, nh, count, u_sieve, f_max, (label=="dense"&&nh<=64)); }, reps);

            std::cout << "nh=" << nh << " | " << std::fixed << std::setprecision(2) 
                      << 100.0*r_base.s.n_pruned/count << "% -> " << 100.0*r_uni.s.n_pruned/count << "%\n";

            f << get_ts() << "," << label << "," << nh << "," 
              << 100.0*r_base.s.n_pruned/count << "," << 100.0*r_adapt.s.n_pruned/count << "," << 100.0*r_uni.s.n_pruned/count << ","
              << r_base.t_sec << "," << r_adapt.t_sec << "," << r_uni.t_sec << "\n";
        }
    };

    std::vector<u64> d_list; for(u64 i=1; i<=256; i++) d_list.push_back(i);
    std::vector<u64> a_list = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096};

    sweep("dense", d_list, d_amt);
    sweep("anchor", a_list, a_amt);

    return 0;
}
