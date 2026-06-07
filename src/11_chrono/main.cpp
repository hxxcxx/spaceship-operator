#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <format>
#include <ratio>
#include <cmath>
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第11章: <chrono>中的日期和时区
// 参考: C++20 Complete Guide Chapter 11
// ============================================================

namespace chr = std::chrono;
using namespace std::literals;

// --- 11.1 通过示例进行概述 ---

// 11.1.1 每月5号安排会议 (year_month_day)
void demo_year_month_day() {
    std::cout << "--- 11.1.1 year_month_day ---\n";

    // 创建日历日期
    chr::year_month_day first = 2021y / 1 / 5;
    for (auto d = first; d.year() == first.year(); d += chr::months{1}) {
        auto tp{chr::sys_days{d} + 18h + 30min};
        std::cout << std::format("  {} -> {:%A %D at %R}\n", d, tp);
    }

    // 初始化日历类型的多种方式
    std::cout << "  初始化方式:\n";
    auto d1 = chr::year{2021} / 1 / 5;       // 2021-01-05
    auto d2 = chr::month{1} / 5 / 2021;       // 2021-01-05
    auto d3 = chr::day{5} / 1 / 2021;         // 2021-01-05
    auto d4 = 2021y / 1 / 5;                  // 2021-01-05
    auto d5 = chr::January / 5 / 2021;        // 2021-01-05
    std::cout << std::format("    d1: {}, d4: {}, d5: {}\n", d1, d4, d5);

    // 访问字段
    std::cout << std::format("  year={}, month={}, day={}\n",
        (int)d1.year(), (unsigned)d1.month(), (unsigned)d1.day());

    std::cout << "\n";
}

// 11.1.2 每月最后一天 (year_month_day_last)
void demo_last_day() {
    std::cout << "--- 11.1.2 year_month_day_last ---\n";

    auto first = 2021y / 1 / chr::last;
    for (auto d = first; d.year() == first.year(); d += chr::months{1}) {
        auto tp{chr::sys_days{d} + 18h + 30min};
        std::cout << std::format("  {} -> {:%D at %R}\n", d, tp);
    }

    std::cout << "\n";
}

// 11.1.3 每月第一个星期一 (year_month_weekday)
void demo_weekday() {
    std::cout << "--- 11.1.3 year_month_weekday ---\n";

    auto first = 2021y / 1 / chr::Monday[1];
    for (auto d = first; d.year() == first.year(); d += chr::months{1}) {
        auto tp{chr::sys_days{d} + 18h + 30min};
        std::cout << std::format("  {} -> {:%A %D at %R}\n", d, tp);
    }

    // 星期几差值
    std::cout << std::format("  Friday - Tuesday = {}\n", chr::Friday - chr::Tuesday);
    std::cout << std::format("  Tuesday - Friday = {}\n", chr::Tuesday - chr::Friday);

    // 月份算术
    auto m = chr::December;
    std::cout << std::format("  Dec + 10months = {}\n", m + chr::months{10});
    std::cout << std::format("  2021y/Dec + 10months = {}\n", 2021y / m + chr::months{10});

    std::cout << "\n";
}

// 11.1.4 时区
void demo_timezones() {
    std::cout << "--- 11.1.4 时区 ---\n";

    try {
        // 当前日期(本地时区)
        auto localNow = chr::current_zone()->to_local(chr::system_clock::now());
        chr::year_month_day today{chr::floor<chr::days>(localNow)};
        std::cout << std::format("  today: {}\n", today);

        // 当年第一个星期一
        auto first = today.year() / 1 / chr::Monday[1];
        auto tp{chr::local_days{first} + 18h + 30min};

        // 本地时区
        chr::zoned_time timeLocal{chr::current_zone(), tp};
        std::cout << std::format("  local: {}\n", timeLocal);

        // 其他时区
        chr::zoned_time timeTokyo{"Asia/Tokyo", timeLocal};
        chr::zoned_time timeNY{"America/New_York", timeLocal};
        std::cout << std::format("  Tokyo: {}\n", timeTokyo);
        std::cout << std::format("  NY:    {}\n", timeNY);

        // 时区信息
        auto tzHere = chr::current_zone();
        auto tzUTC = chr::locate_zone("UTC");
        std::cout << std::format("  current zone: {}\n", tzHere->name());
        std::cout << std::format("  UTC zone:     {}\n", tzUTC->name());

        // sys_time vs local_time 区别
        auto sysNow = chr::floor<chr::seconds>(chr::system_clock::now());
        auto locNow = chr::current_zone()->to_local(sysNow);
        auto timeFromSys = chr::zoned_time{chr::current_zone(), sysNow};  // 转换
        auto timeFromLoc = chr::zoned_time{chr::current_zone(), locNow};  // 应用
        std::cout << std::format("  sys->local: {}\n", timeFromSys);
        std::cout << std::format("  loc->local: {}\n", timeFromLoc);

    } catch (const std::exception& e) {
        std::cout << std::format("  [时区不支持] {}\n", e.what());
    }

    std::cout << "\n";
}

// --- 11.3 C++20中的基本chrono扩展 ---

// 11.3.1 新的持续时间类型
void demo_new_durations() {
    std::cout << "--- 11.3.1 新的持续时间类型 ---\n";

    chr::days d{7};
    chr::weeks w{2};
    chr::months m{3};
    chr::years y{1};

    std::cout << std::format("  days:   {}\n", d);
    std::cout << std::format("  weeks:  {}\n", w);
    std::cout << std::format("  months: {}\n", m);
    std::cout << std::format("  years:  {}\n", y);

    // 注意: months/years 是平均持续时间
    // months = 30.436875天, years = 365.2425天
    std::cout << "  注意: months和years是平均持续时间(30.44天/365.24天)\n";
    std::cout << "  对日历类型加months正确处理月末\n";
    std::cout << "  对sys_days加months得到平均偏移(可能不是期望日期)\n";

    std::cout << "\n";
}

// 11.3.2 时钟和时间点类型
void demo_clocks_timepoints() {
    std::cout << "--- 11.3.2 时钟和时间点类型 ---\n";

    // 系统时钟
    auto sysNow = chr::system_clock::now();
    auto sysDays = chr::floor<chr::days>(sysNow);
    std::cout << std::format("  system_clock: {:%F %T}\n", sysNow);
    std::cout << std::format("  sys_days:     {}\n", sysDays);

    // UTC时钟
    auto utcNow = chr::utc_clock::now();
    std::cout << std::format("  utc_clock:    {:%F %T %Z}\n", utcNow);

    // GPS时钟
    auto gpsNow = chr::gps_clock::now();
    std::cout << std::format("  gps_clock:    {:%F %T %Z}\n", gpsNow);

    // TAI时钟
    auto taiNow = chr::tai_clock::now();
    std::cout << std::format("  tai_clock:    {:%F %T %Z}\n", taiNow);

    // 精度向下取整
    auto tpSec = chr::floor<chr::seconds>(sysNow);
    auto tpMin = chr::floor<chr::minutes>(sysNow);
    auto tpHr = chr::floor<chr::hours>(sysNow);
    auto tpDay = chr::floor<chr::days>(sysNow);
    std::cout << std::format("  floor<seconds>: {}\n", tpSec);
    std::cout << std::format("  floor<minutes>: {}\n", tpMin);
    std::cout << std::format("  floor<hours>:   {}\n", tpHr);
    std::cout << std::format("  floor<days>:    {}\n", tpDay);

    // is_clock检查
    std::cout << std::format("  is_clock<system_clock>: {}\n",
        chr::is_clock_v<chr::system_clock>);
    std::cout << std::format("  is_clock<local_t>:      {}\n",
        chr::is_clock_v<chr::local_t>);

    std::cout << "\n";
}

// 11.3.5 hh_mm_ss
void demo_hh_mm_ss() {
    std::cout << "--- 11.3.5 hh_mm_ss ---\n";

    // 从当前时间提取时分秒
    auto now = chr::system_clock::now();
    auto today = chr::floor<chr::days>(now);
    chr::hh_mm_ss hms{now - today};

    std::cout << std::format("  hms:      {}\n", hms);
    std::cout << std::format("  hours:    {}\n", hms.hours());
    std::cout << std::format("  minutes:  {}\n", hms.minutes());
    std::cout << std::format("  seconds:  {}\n", hms.seconds());
    std::cout << std::format("  subsecs:  {}\n", hms.subseconds());
    std::cout << std::format("  negative: {}\n", hms.is_negative());

    // 其他精度
    chr::hh_mm_ss h1{10000s};
    std::cout << std::format("  10000s -> {}\n", h1);

    chr::duration<int, std::ratio<1, 3>> third{1};
    chr::hh_mm_ss h2{third};
    std::cout << std::format("  1/3s -> {}\n", h2);

    // 格式化输出持续时间
    std::cout << std::format("  10000s format: {:%T}\n", 10000s);
    std::cout << std::format("  -10000s format: {:%T}\n", -10000s);

    std::cout << "\n";
}

// 11.3.6 小时相关实用函数
void demo_hour_utils() {
    std::cout << "--- 11.3.6 小时实用函数 ---\n";

    chr::hours h9{9};
    chr::hours h17{17};

    std::cout << std::format("  09:00 is_am: {}, make12: {}pm\n",
        chr::is_am(h9), chr::make12(h9).count());
    std::cout << std::format("  17:00 is_am: {}, is_pm: {}, make12: {}pm\n",
        chr::is_am(h17), chr::is_pm(h17), chr::make12(h17).count());

    // make24: 第二个参数true表示PM
    chr::hours h5pm{5};
    auto h24 = chr::make24(h5pm, true);
    std::cout << std::format("  5pm -> make24: {}\n", h24.count());

    std::cout << "\n";
}

// --- 11.4 chrono类型的输入输出 ---

// 11.4.1 默认输出格式
void demo_default_output() {
    std::cout << "--- 11.4.1 默认输出格式 ---\n";

    // 日历类型默认输出
    std::cout << std::format("  day:       {}\n", chr::day{5});
    std::cout << std::format("  month:     {}\n", chr::month{2});
    std::cout << std::format("  year:      {}\n", chr::year{2021});
    std::cout << std::format("  weekday:   {}\n", chr::weekday{3});
    std::cout << std::format("  ymd:       {}\n", 2021y / 2 / 5);
    std::cout << std::format("  yml:       {}\n", 2021y / 2 / chr::last);
    std::cout << std::format("  ymw:       {}\n", 2021y / 2 / chr::Monday[1]);

    // 持续时间
    std::cout << std::format("  42ms:      {}\n", 42ms);
    std::cout << std::format("  1.5s:      {}\n", 1.5s);
    std::cout << std::format("  7d:        {}\n", chr::days{7});

    // 时间点
    auto now = chr::system_clock::now();
    std::cout << std::format("  now:       {}\n", now);

    std::cout << "\n";
}

// 11.4.2 格式化输出
void demo_format_output() {
    std::cout << "--- 11.4.2 格式化输出 ---\n";

    chr::sys_days d = 2021y / 6 / 9;
    auto tp = chr::sys_days{d} + 17h + 33min + 16s + chr::milliseconds{850};

    // 日期格式
    std::cout << std::format("  %c:  {:%c}\n", tp);
    std::cout << std::format("  %F:  {:%F}\n", tp);
    std::cout << std::format("  %D:  {:%D}\n", tp);
    std::cout << std::format("  %x:  {:%x}\n", tp);

    // 时间格式
    std::cout << std::format("  %T:  {:%T}\n", tp);
    std::cout << std::format("  %R:  {:%R}\n", tp);
    std::cout << std::format("  %X:  {:%X}\n", tp);

    // 星期
    std::cout << std::format("  %A:  {:%A}\n", tp);
    std::cout << std::format("  %a:  {:%a}\n", tp);
    std::cout << std::format("  %w:  {:%w}\n", tp);
    std::cout << std::format("  %u:  {:%u}\n", tp);

    // 月份
    std::cout << std::format("  %B:  {:%B}\n", tp);
    std::cout << std::format("  %b:  {:%b}\n", tp);
    std::cout << std::format("  %m:  {:%m}\n", tp);

    // 年
    std::cout << std::format("  %Y:  {:%Y}\n", tp);
    std::cout << std::format("  %y:  {:%y}\n", tp);

    // 周
    std::cout << std::format("  %W:  {:%W}\n", tp);
    std::cout << std::format("  %V:  {:%V}\n", tp);
    std::cout << std::format("  %j:  {:%j}\n", tp);

    // 持续时间格式
    auto dur = 10000s;
    std::cout << std::format("  dur %T: {:%T}\n", dur);
    std::cout << std::format("  dur %S: {:%S}\n", dur);
    std::cout << std::format("  dur %q: {:%q}\n", dur);
    std::cout << std::format("  dur %Q: {:%Q}\n", dur);

    // 日期+时间组合
    std::cout << std::format("  full: {:%A, %B %d, %Y at %R}\n", tp);

    std::cout << "\n";
}

// 11.4.4 格式化输入
void demo_parse() {
    std::cout << "--- 11.4.4 格式化输入 ---\n";

    // from_stream 解析时间点
    chr::sys_seconds tp;
    std::istringstream sstrm1{"2021-2-28 17:30:00"};
    chr::from_stream(sstrm1, "%F %T", tp);
    if (sstrm1) {
        std::cout << std::format("  from_stream: {}\n", tp);
    }

    // 解析 year_month
    chr::year_month ym;
    std::istringstream sstrm2{"Monday, April 5, 2021"};
    chr::from_stream(sstrm2, "%A, %B %d, %Y", ym);
    if (sstrm2) {
        std::cout << std::format("  year_month: {}\n", ym);
    }

    // parse 操纵符
    chr::sys_days pDay;
    chr::hours pH;
    chr::minutes pM;
    std::istringstream{"12/24/21 18:00"}
        >> chr::parse(std::string{"%D"}, pDay)
        >> chr::parse(std::string{" %H"}, pH)
        >> chr::parse(std::string{":%M"}, pM);
    std::cout << std::format("  parse: {} {} {}\n", pDay, pH, pM);

    // 解析带时区
    try {
        chr::local_seconds ltp;
        std::string tzAbbrev;
        std::istringstream sstrm3{"2021-4-13 12:00 MST"};
        sstrm3 >> chr::parse(std::string{"%F %R %Z"}, ltp, tzAbbrev);
        if (sstrm3.good()) {
            std::cout << std::format("  local+tz: {:%F %R} {}\n", ltp, tzAbbrev);
        }
    } catch (const std::exception& e) {
        std::cout << std::format("  parse tz error: {}\n", e.what());
    }

    std::cout << "\n";
}

// --- 11.5 实际使用 ---

// 11.5.1 无效日期
void demo_invalid_dates() {
    std::cout << "--- 11.5.1 无效日期 ---\n";

    // 无效初始化
    chr::day d0{0};
    chr::year_month_day ymd{2021y / 2 / 31};
    std::cout << std::format("  invalid day: {}\n", d0);
    std::cout << std::format("  invalid ymd: {}\n", ymd);

    // ok() 检查
    std::cout << std::format("  2021/2/31 ok: {}\n", ymd.ok());
    std::cout << std::format("  2021/2/28 ok: {}\n", (2021y / 2 / 28).ok());
    std::cout << std::format("  2020/2/29 ok: {} (闰年)\n", (2020y / 2 / 29).ok());

    // 计算产生无效日期
    auto ymd1 = chr::year{2021} / 1 / 31;
    ymd1 += chr::months{1};  // 2021/2/31 无效!
    std::cout << std::format("  Jan31 + 1month: {} ok={}\n", ymd1, ymd1.ok());

    // 处理方式1: 向下舍入到月末
    if (!ymd1.ok()) {
        ymd1 = ymd1.year() / ymd1.month() / chr::last;
    }
    std::cout << std::format("  舍入到月末: {}\n", ymd1);

    // 处理方式2: 溢出到下月(通过sys_days)
    auto ymd2 = chr::year{2021} / 1 / 31 + chr::months{1};
    if (!ymd2.ok()) {
        ymd2 = chr::sys_days{ymd2};  // 溢出处理
    }
    std::cout << std::format("  溢出到下月: {}\n", ymd2);

    // month_day 有效性
    std::cout << std::format("  Feb/29 ok: {}\n", chr::month_day{chr::February, chr::day{29}}.ok());
    std::cout << std::format("  Feb/30 ok: {}\n", chr::month_day{chr::February, chr::day{30}}.ok());

    // 格式化不标记无效
    chr::year_month_day bad{2021y / 2 / 31};
    std::cout << std::format("  default: {}\n", bad);          // 标记无效
    std::cout << std::format("  %F:     {:%F}\n", bad);        // 不标记

    std::cout << "\n";
}

// 11.5.2 处理月份和年份
void demo_month_year_arithmetic() {
    std::cout << "--- 11.5.2 月份和年份算术 ---\n";

    // year_month_day + months: 逻辑日期
    chr::year_month_day ymd0 = chr::year{2020} / 12 / 31;
    auto ymd1 = ymd0 + chr::months{4};
    auto ymd2 = ymd0 + chr::years{4};
    std::cout << std::format("  ymd: {}\n", ymd0);
    std::cout << std::format("    +4months: {} (可能无效)\n", ymd1);
    std::cout << std::format("    +4years:  {}\n", ymd2);

    // year_month_day_last + months: 自动调整到月末
    chr::year_month_day_last yml0 = chr::year{2020} / 12 / chr::last;
    auto yml1 = yml0 + chr::months{4};
    std::cout << std::format("  yml: {}\n", yml0);
    std::cout << std::format("    +4months: {} -> {}\n", yml1, chr::sys_days{yml1});

    // sys_days + months: 平均偏移(不推荐!)
    chr::sys_days day0 = chr::year{2020} / 12 / 31;
    auto day1 = day0 + chr::months{4};
    auto day2 = day0 + chr::years{4};
    std::cout << std::format("  sys_days: {}\n", day0);
    std::cout << std::format("    +4months: {} (平均偏移!)\n", day1);
    std::cout << std::format("    +4years:  {} (平均偏移!)\n", day2);

    // 推荐用 weeks 替代
    auto day3 = day0 + chr::weeks{4};
    std::cout << std::format("    +4weeks:  {} (精确)\n", day3);

    std::cout << "\n";
}

// 11.5.3 解析时间点属性
void demo_time_attributes() {
    std::cout << "--- 11.5.3 时间点属性 ---\n";

    auto now = chr::system_clock::now();
    auto today = chr::floor<chr::days>(now);
    chr::year_month_day ymd{today};
    chr::hh_mm_ss hms{now - today};
    chr::weekday wd{today};

    std::cout << std::format("  now:     {}\n", now);
    std::cout << std::format("  today:   {}\n", today);
    std::cout << std::format("  ymd:     {}\n", ymd);
    std::cout << std::format("  hms:     {}\n", hms);
    std::cout << std::format("  year:    {}\n", ymd.year());
    std::cout << std::format("  month:   {}\n", ymd.month());
    std::cout << std::format("  day:     {}\n", ymd.day());
    std::cout << std::format("  hours:   {}\n", hms.hours());
    std::cout << std::format("  minutes: {}\n", hms.minutes());
    std::cout << std::format("  seconds: {}\n", hms.seconds());
    std::cout << std::format("  weekday: {}\n", wd);

    try {
        chr::sys_info info{chr::current_zone()->get_info(now)};
        std::cout << std::format("  tz abbrev: {}\n", info.abbrev);
        std::cout << std::format("  tz offset: {}\n", info.offset);
    } catch (const std::exception& e) {
        std::cout << std::format("  [无时区] {}\n", e.what());
    }

    std::cout << "\n";
}

// --- 11.6 时区详解 ---
void demo_timezone_detail() {
    std::cout << "--- 11.6 时区详解 ---\n";

    try {
        // 时区数据库
        std::cout << std::format("  tzdb version: {}\n", chr::get_tzdb().version);
        std::cout << std::format("  zones count:  {}\n", chr::get_tzdb().zones.size());

        // 当前时区
        auto tzHere = chr::current_zone();
        std::cout << std::format("  current zone: {}\n", tzHere->name());

        // locate_zone
        auto tzTokyo = chr::locate_zone("Asia/Tokyo");
        auto tzNY = chr::locate_zone("America/New_York");
        std::cout << std::format("  Tokyo zone: {}\n", tzTokyo->name());
        std::cout << std::format("  NY zone:    {}\n", tzNY->name());

        // zoned_time: 应用 vs 转换
        auto day = 2021y / 9 / chr::Friday[chr::last];
        chr::local_seconds tpOffice{chr::local_days{day} - 6h};  // 前18:00
        chr::sys_seconds tpCompany{chr::sys_days{day} + 17h};     // UTC 17:00

        std::cout << "  Berlin office/company:\n";
        std::cout << std::format("    {}\n", chr::zoned_time{"Europe/Berlin", tpOffice});
        std::cout << std::format("    {}\n", chr::zoned_time{"Europe/Berlin", tpCompany});

        std::cout << "  NY office/company:\n";
        std::cout << std::format("    {}\n", chr::zoned_time{"America/New_York", tpOffice});
        std::cout << std::format("    {}\n", chr::zoned_time{"America/New_York", tpCompany});

        // 时区缩写查找 (CST有多个含义)
        auto dayCST = chr::sys_days{2021y / 1 / 1};
        int cstCount = 0;
        for (const auto& z : chr::get_tzdb().zones) {
            if (z.get_info(dayCST).abbrev == "CST") {
                if (cstCount < 5) {
                    chr::zoned_time zt{&z, dayCST};
                    std::cout << std::format("  CST: {} ({})\n", zt, z.name());
                }
                cstCount++;
            }
        }
        std::cout << std::format("  CST total: {} entries\n", cstCount);

    } catch (const std::exception& e) {
        std::cout << std::format("  [时区不支持] {}\n", e.what());
    }

    std::cout << "\n";
}

// --- 11.7 时钟详解 ---

// 11.7.3 闰秒
void demo_leap_seconds() {
    std::cout << "--- 11.7.3 闰秒 ---\n";

    // 2016-12-31 23:59:60 UTC 是闰秒
    auto tpUtc = chr::clock_cast<chr::utc_clock>(chr::sys_days{2017y / 1 / 1} - 1000ms);
    std::cout << "  闰秒附近:\n";
    for (auto end = tpUtc + 2500ms; tpUtc <= end; tpUtc += 500ms) {
        auto tpSys = chr::clock_cast<chr::system_clock>(tpUtc);
        auto tpGps = chr::clock_cast<chr::gps_clock>(tpUtc);
        auto tpTai = chr::clock_cast<chr::tai_clock>(tpUtc);
        std::cout << std::format("    SYS {:%H:%M:%S}  UTC {:%H:%M:%S}  GPS {:%H:%M:%S}  TAI {:%H:%M:%S}\n",
            tpSys, tpUtc, tpGps, tpTai);
    }

    std::cout << "\n";
}

// 11.7.4 时钟间转换
void demo_clock_conversions() {
    std::cout << "--- 11.7.4 时钟间转换 ---\n";

    auto now = chr::system_clock::now();

    // system -> UTC/GPS/TAI
    auto utcTp = chr::clock_cast<chr::utc_clock>(now);
    auto gpsTp = chr::clock_cast<chr::gps_clock>(now);
    auto taiTp = chr::clock_cast<chr::tai_clock>(now);

    std::cout << std::format("  system: {:%F %T}\n", now);
    std::cout << std::format("  UTC:    {:%F %T %Z}\n", utcTp);
    std::cout << std::format("  GPS:    {:%F %T %Z}\n", gpsTp);
    std::cout << std::format("  TAI:    {:%F %T %Z}\n", taiTp);

    // 闰秒转换示例
    chr::utc_time<chr::utc_clock::duration> tpLeap;
    std::istringstream{"2015-6-30 23:59:60"} >> chr::parse(std::string{"%F %T"}, tpLeap);
    auto tpLeapSys = chr::clock_cast<chr::system_clock>(tpLeap);
    auto tpLeapGps = chr::clock_cast<chr::gps_clock>(tpLeap);
    auto tpLeapTai = chr::clock_cast<chr::tai_clock>(tpLeap);

    std::cout << std::format("  leap UTC:  {:%F %T %Z}\n", tpLeap);
    std::cout << std::format("  leap SYS:  {:%F %T}\n", tpLeapSys);
    std::cout << std::format("  leap GPS:  {:%F %T %Z}\n", tpLeapGps);
    std::cout << std::format("  leap TAI:  {:%F %T %Z}\n", tpLeapTai);

    std::cout << "\n";
}

// 11.6.5 自定义时区 (OffsetZone)
class OffsetZone {
    chr::minutes offset;
public:
    explicit OffsetZone(chr::minutes offs) : offset{offs} {}

    template<typename Duration>
    auto to_local(chr::sys_time<Duration> tp) const {
        using LT = chr::local_time<std::common_type_t<Duration, chr::minutes>>;
        return LT{(tp + offset).time_since_epoch()};
    }

    template<typename Duration>
    auto to_sys(chr::local_time<Duration> tp) const {
        using ST = chr::sys_time<std::common_type_t<Duration, chr::minutes>>;
        return ST{(tp - offset).time_since_epoch()};
    }

    template<typename Duration>
    auto get_info(const chr::sys_time<Duration>&) const {
        return chr::sys_info{};
    }
};

void demo_custom_timezone() {
    std::cout << "--- 11.6.5 自定义时区 ---\n";

    OffsetZone p3_45{3h + 45min};
    auto now = chr::system_clock::now();
    chr::zoned_time<decltype(now)::duration, OffsetZone*> zt{&p3_45, now};

    std::cout << std::format("  UTC:   {}\n", zt.get_sys_time());
    std::cout << std::format("  +3:45: {}\n", zt.get_local_time());

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第11章: <chrono>中的日期和时区 ===\n\n";

    demo_year_month_day();
    demo_last_day();
    demo_weekday();
    demo_timezones();
    demo_new_durations();
    demo_clocks_timepoints();
    demo_hh_mm_ss();
    demo_hour_utils();
    demo_default_output();
    demo_format_output();
    demo_parse();
    demo_invalid_dates();
    demo_month_year_arithmetic();
    demo_time_attributes();
    demo_timezone_detail();
    demo_leap_seconds();
    demo_clock_conversions();
    demo_custom_timezone();

    return 0;
}
