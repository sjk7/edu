#pragma once
#ifndef DPS_DPS_LIB_H_INCLUDED
#define DPS_DPS_LIB_H_INCLUDED

#pragma pack(push, 1)
#ifdef _MSC_VER
#	pragma warning(disable: 26495)
#	pragma warning(disable: 26481)
#	pragma warning(disable: 26493) // casts warning
#	pragma warning(disable: 26491) // casts warning
#endif

#include <cstdint>
#include <string>
#include <vector>
#include "../sjk/sjk_exception.h"
#include "../sjk/sjk_file.h"
#include "../sjk/sjk_strings.h"
#include <set>

namespace dps
{
	static const int CAT_MAX = 50;
	static const int Max_Path = 260;
	static const int ART_MAX = 50;
	static const int ALB_MAX = 80;
	/*/
	Public Const CAT_MAX As Long = 50

	Public Type tFixedTonInfoDPS
	Sectone As Double
	cueIn As Double
	DropStart As Double
	DropStop As Double
	Intro As Double
	Endstyle As String * 1
	Speed As Double
	SoftDump As Double
	category As String * CAT_MAX
	CatLevel As Integer
	Opener As Boolean
	Tert As Double
	Artist As String * 50
	Title As String * 50
	Year As String * 4
	HardDiskLoc As String * 250 ' MAX_Path - 10
	fileName As String * MAX_PATH
	FilePath As String * MAX_PATH
	Spare1 As Double
	STAR As String * 1
	Volume As Long
	Outro As Double
	TrackedStart As Double
	Trackedfiles As String * 209 'artistb4 " - " & Titleb4 ",,," & ArtistAfter & " - " & TitleAfter
	Duration As Double
	LastDateScheduled As String * 10
	ActualDuration As Double ' DONT rely on this to be right: you should always calculate it. See Function ActualDurationForTon (its (sectone / speed) - cuein))
	DisplayDuration As String * 10
	MarkedAsErased As Boolean
	IsPopulated As Boolean
	ArrayIndex As Long
	LineBreak As String * 2

	End Type

	Public Type tFixedTonInfoEx
	t As tFixedTonInfoDPS
	Album As String * 80
	SpareString As String * 50
	SpareLong As Long

	End Type
	/*/
	struct TFIXEDTONINFODPS
	{
		double sectone;
		double cuein;
		double dropstart;
		double dropstop;
		double intro;
		char endstyle;
		double speed;
		double softdump;
		char category[CAT_MAX];
		int16_t catlevel;
		int16_t opener;
		double tert;
		char artist[CAT_MAX];
		char title[CAT_MAX];
		char year[4];
		char harddiskloc[250];
		char filename[Max_Path];
		char filepath[Max_Path];
		double spare1;
		char star;
		int32_t volume;
		double outro;
		double trackedstart;
		char trackedfiles[209];
		double duration;
		char lastdatescheduled[10];
		double actualduration;
		char displayduration[10];
		int16_t markedaserased;
		int16_t ispopulated;
		int32_t arrayindex;
		char linebreak[2];
	};

	struct TFIXEDTONINFOEX
	{
		TFIXEDTONINFODPS toninfo;
		char album[80];
		char spare[50];
		int32_t spareint;
	};

#	pragma pack(pop)

	using tex = TFIXEDTONINFOEX;
	using tvec_t = std::vector<tex>;
	struct dps_lib
	{
		dps_lib(std::string path) : m_spath(path) {

			m_f.open(m_spath, std::ios_base::in | std::ios_base::binary);
			if (!m_f) {
				SJK_EXCEPTION("Cannot open dps lib at: ", m_spath);
			}

			const size_t sz = sizeof(tex);

			const double fsize = static_cast<double>(m_f.size_bytes());
			const double ds = (double)sz;
			const double div = fsize / ds;
			const int idiv = static_cast<int>(div);
			if ( (double)idiv != div ) {
				SJK_EXCEPTION("DPS lib size mismatch for file: ", m_spath, idiv, div);
			}
			m_vec.resize(size_t(idiv));
			
			sjk::span<char> sp((char*)m_vec.data(), m_vec.size() * sizeof(tex));
			assert(sp.size_bytes() == idiv * sizeof(tex));
			const int64_t actual_read = m_f.read(sp);
                        assert(static_cast<size_t>(actual_read) == m_vec.size() * sizeof(tex)); (void)actual_read;

		}

		std::string m_spath;
		sjk::file m_f;
		tvec_t m_vec;
		
	};

	static inline void string_from_dps(const char* from, const size_t sz_from,
									   char* to, const size_t sz_to)
	{
		using namespace sjk::str;
		cstring_to_cstring(from, sz_from, to, sz_to);
		rtrim_cstring(to);
	}
	static const char* CATSEP = "/";
	static inline sjk::str::vec_t cats_from_string(std::string& s)
	{
		sjk::str::vec_t ret;
		if (s.empty()) return sjk::str::vec_t{ "NOCAT" };
		sjk::str::to_upper(s);
		const auto f = s.find(CATSEP);
		if (f == std::string::npos){
			ret.push_back(s);
		}
		else {
			sjk::str::split(CATSEP, s, ret);
		}
		return ret;
	}
	using catset_t = std::set<std::string>;

	static inline catset_t cat_set_from_vec(const sjk::str::vec_t& v) {
		catset_t set(v.begin(), v.end());
		if (set.size() > 1) {
			set.erase("NOCAT");
		}
		return set;
	}



}


#endif //#ifndef DPS_DPS_LIB_H_INCLUDED
