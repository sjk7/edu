#ifndef PL_TYPES_H
#define PL_TYPES_H


#include "../sjk/sjk_db_cache.h"
#include "../sjk/sjk_strings.h"
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <ostream>
#include <iostream>
#include <initializer_list>
#include <cassert>
#include <array>

namespace playout
{
	//////////////////////////////////
#pragma pack(push, 1)
#ifdef _MSC_VER
#	pragma warning(disable: 26495)
#	pragma warning(disable: 26481)
#endif

	using TONE_TYPE = uint32_t;
	static constexpr auto TONE_NOT_SET = static_cast<TONE_TYPE>(~0);
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
	struct strings
	{
		static constexpr int MAX_STR = 101;
		static constexpr int MAX_CAT_LEN = 101;
		static constexpr int MAX_NUM_CATS = 32;
		static constexpr int MAX_FILE_PATH = MAX_PATH;
		using level_type = uint16_t;
		static constexpr level_type LEVEL_INVALID = level_type(~0);
		static constexpr level_type LEVEL_MAX = LEVEL_INVALID - 1;
		using strvec_type = std::vector<std::string>;


		char artist			[MAX_STR]		{0};
		char title			[MAX_STR]		{0};
		char path			[MAX_FILE_PATH]	{0};
		char album			[MAX_STR]		{0};
		char composer		[MAX_STR]		{0};
		char publisher		[MAX_STR]		{0};
		char comments		[MAX_STR]		{0};
                uint8_t OPENER_NOT_SET = 0;
                uint8_t opener          [MAX_NUM_CATS] {OPENER_NOT_SET};

		// char categories		[MAX_CAT_LEN][MAX_NUM_CATS] {{0}};
		using cat_array_type = std::array < std::array <char, MAX_CAT_LEN > , MAX_NUM_CATS >;
		cat_array_type categories;
		level_type levels	[MAX_NUM_CATS] {LEVEL_INVALID};

		template <typename R, typename IDX, typename CATVEC, typename CATS>
		static inline void pop_cats_from_record(const R& r,
												const IDX& idx,
												CATVEC& catvec,
												CATS& cats)
		{
			cats.clear();
			ASSERT(cats.capacity() >= strings::MAX_NUM_CATS);

			for (int icat = 0; icat < strings::MAX_NUM_CATS; icat++)
			{
				const auto& a = r.m_data.strvals.categories[icat];
				if (strlen(a.data())){
					cats.emplace_back(std::string(a.data()));
					// cats.insert(std::string(a.data()));
				}else{
					break;
				}
			}

			cats.sort();
			// yep, even if its empty, we expect an entry there, else it will be awkward to add stuff later
			catvec.emplace_back(std::make_pair(cats, idx));

		}
	};
#ifdef _MSC_VER
#	pragma warning(default: 26495)
#	pragma warning(default: 24681)
#endif

	template <typename T, size_t S = 1024UL>
	struct blob
	{
		using type = T;
		constexpr size_t size() const{ return S;}
		T data[S] = { 0 };
	};

	template <typename T>
	struct head_tail
	{
		using SFIANE = T;
		head_tail(std::initializer_list<T> li) noexcept
		{
			const auto sz = li.size();
			if (sz) {
				m_head = *(li.begin());
			}
			if (sz >= 2) {
				m_tail = *(li.begin() + 1);
			}
		}

		head_tail() {}
		T m_head  {TONE_NOT_SET};
		T m_tail  {TONE_NOT_SET};

		// In Win, this was causing ambiguity for std::string operator <<. SFIANE'd, therefore
		template <typename V, typename U = typename V::const_iterator,
				  typename Z = typename V::value_type::SFIANE>
		friend std::ostream& operator<< (std::ostream& os, const V& v)
		{
			for (const auto& tone : v){
				print_tone(os, tone);
			}
			return os;
		}

		friend std::ostream& operator<<(std::ostream& os, const head_tail& ht){
			print_tone(os, ht);
			return os;
		}

		friend std::ostream& print_tone(std::ostream& os, const head_tail& ht) {
			print_tone(os, ht, true);
			print_tone(os, ht, false);
			os << "\n";
			return os;
		}

		friend std::ostream& print_tone(std::ostream& os, const head_tail& ht, int is_head)
		{
			if (is_head) {
				if (ht.m_head == TONE_NOT_SET)
				{ os << "head: " << "NOT SET";}
				else { os << "head: " << ht.m_head; }
			}else {
				if (ht.m_tail == TONE_NOT_SET)
				{ os << " tail: " << "NOT SET";
				}else {	os << " tail: " << ht.m_tail;}
			}
			return os;
		}
	};


	/*/
	template <typename T>
	std::ostream& operator << (std::ostream& os, const& std::pair<head_tail<T> pr)
	{
		os << "index: " << pr.first << " " << pr.second << std::endl;
		return os;
	}
	/*/

#ifdef _MSC_VER
#pragma warning(disable: 26495)
#endif
	struct tones
	{
		static constexpr uint8_t MAX_INTROS{ 8 };
		static constexpr uint8_t MAX_SECTONES{ 8 };
		using type = TONE_TYPE;
		static const type NOT_SET = TONE_NOT_SET;
		using VOLUME_TYPE = int32_t;

		static constexpr VOLUME_TYPE  VOLUME_NOT_SET = std::numeric_limits<VOLUME_TYPE>::min();
		static constexpr VOLUME_TYPE VOLUME_MAX = 0;
		static constexpr VOLUME_TYPE VOLUME_MIN = -100000; // -100dB
		typedef head_tail<type> value_type;

		using greater = typename std::greater<type>;
		using less = typename std::less<type>;
		value_type intros[MAX_INTROS]{ {} };
		value_type sectones[MAX_SECTONES]{ {} };

		type cue_in { NOT_SET };
		type duration{ NOT_SET };
                static constexpr type DURATION_UNKNOWN = NOT_SET;
                static constexpr type STREAM_DURATION = DURATION_UNKNOWN;
		VOLUME_TYPE volume { VOLUME_NOT_SET };

                using SPEED_TYPE = int8_t;
                static constexpr SPEED_TYPE SPEED_NOT_SET = 0;
                SPEED_TYPE speed {SPEED_NOT_SET};

		friend std::ostream& operator<<(std::ostream& os,  tones& ht) {
			os << "Intros: " << "\n";
			sjk::span<value_type> sp (&ht.intros[0], MAX_INTROS);
			assert(sp.size() == MAX_INTROS);

			for (size_t i = 0; i < MAX_INTROS; i++) {
				os << sp[i];
			}
			os << "Sectones: " << "\n";
			sjk::span<value_type> sp2(&ht.sectones[0], MAX_SECTONES);
			assert(sp2.size() == MAX_SECTONES);
			for (size_t i = 0; i < MAX_SECTONES; i++) {
				os << sp2[i];
			}
			return os;
		}
	};

	typedef std::vector<tones::value_type> tonevec_t;

	template <typename T>
	static inline std::ostream& operator << (std::ostream& os, const tonevec_t& v)
	{
		for (auto& ht : v){
			os << ht;
		}
		return os;
	}


	using tvt_t = tones::value_type;
	static inline bool operator == (const tvt_t& left, const tvt_t& right) {
		return right.m_head == left.m_head && right.m_tail == left.m_tail;
	}

	static inline bool operator < (const tvt_t& left, const tvt_t& right) {
		return right.m_head < left.m_head;
	}
	static  inline bool operator > (const tvt_t& left, const tvt_t& right) {
		return right.m_head > left.m_head;
	}


#ifdef _MSC_VER
#pragma warning(default: 26495)
#endif

	struct rec
	{
		strings strvals = {};
		tones tonevals = {};
		uint16_t year = {0};
	};
#pragma pack(pop)
	///////////////////////////////// align pragma ends /////////////////////////////////////

	template <typename CMP = tones::less >
	static inline bool compare_multi_tones(const tonevec_t& v1, const tonevec_t& v2)
	{
		CMP c;
		const size_t cnt = std::min(v1.size(), v2.size());
		ASSERT(v1.size() == v2.size()); // need to handle the case where the vectors may not
		// be of equal size, but we still need to know if *one* value in the larger-sized vector
		// is greater (or less) than any value in the smaller one.
		for (size_t i = 0; i < cnt; i++)
		{
			auto& ht1 = v1[i]; auto& ht2 = v2[i];
			// I think its enough to check where *any* head is (nominally) less (or greater) than any other head
			return c(ht1.m_head, ht2.m_head);
		}
		return false;
	}
	// compare two vectors of multiple tones
	static inline bool operator <(const tonevec_t& v1, const tonevec_t& v2)
	{
		return compare_multi_tones(v1, v2);
	}

	static inline bool operator >(const tonevec_t& v1, const tonevec_t& v2)
	{
		return compare_multi_tones<tones::greater>(v1, v2);
	}


	using sortable = sjk::sortable;
	template <typename V>
	static inline bool compare_multi_tones(const V& v1, const V& v2, sortable::sortorder so)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		if (so == sortable::sortorder::asc){
			return compare_multi_tones(v1, v2);
		}else{
			return compare_multi_tones(v1, v2, std::greater<int32_t>());
		}
	}

	template <typename T>
	sjk::span<T> span_from_tones(T* tones, const size_t size)
	{
		return sjk::span<T>(&tones[0], size);
	}

	using so_t = sjk::sortable::sortorder;
	// sorting the values when each index entry has one item:
	template <typename X, typename V, typename U = X>
	static inline void sort_by_pair_first(V& vec, const so_t so,
										  typename std::enable_if<!sjk::collections::is_vector<U>::value>::type* = 0){
		sjk::collections::sort_by_pair_first(vec, so == so_t::asc);
	}

	// sorting the values when each index entry can have multiple items:
	template <typename X, typename V, typename = std::enable_if<sjk::collections::is_vector<X>::value>>
	static inline void sort_by_pair_first(V& vec, const so_t so)
	{
		auto& v = vec;
		std::stable_sort(v.begin(), v.end(),
						 [&](const auto& p1, const auto& p2){

			return compare_multi_tones(p1.first, p2.first, so);
		});
	}


} // namespace playout

#endif // PL_TYPES_H
