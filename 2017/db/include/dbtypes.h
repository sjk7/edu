// dbtypes.h, by Steve. Open Source with no restrictions. Sat 18th Feb 2017. Hope it helps you!
#pragma once
#ifndef DBYPES_H
#define DBYPES_H

#include "runtime_error.h"
#include <cstdint>
#include <string>
#include <map>
#include <limits>
#include "num.h"

namespace cpp
{
	namespace db
	{
		struct tt_tag : public cpp::tag_base<int64_t>{};
		using time_t = cpp::num<tt_tag>;

		#define ROW_INVALID -1
		struct rw_tag : public cpp::tag_base<int32_t, ROW_INVALID>{};
		using rowidx_t = cpp::num<rw_tag>;

		#define UID_INVALID -1
		struct uid_tag : public cpp::tag_base<int32_t, UID_INVALID>{};
		using uid_t = cpp::num<uid_tag>;

		using valpr_t = std::pair<rowidx_t, std::string>;
		using std::ostream;
		static constexpr int16_t NUM_DEFAULT_COLUMNS = 3;
		static constexpr size_t USE_DEFAULT_WIDTH = static_cast<size_t>(-1);
		static constexpr size_t DEFAULT_STRING_WIDTH = 256;


		struct column_types {

			enum  TYPE {BAD = 0, INT = 1, UINT = 2, STRING = 4, INT64 = 8, DATE = INT64, ARRAY = 16};

			using map_t = std::map<std::string, TYPE>;
			static map_t& type_map()
			{
				static map_t m;

				if (m.empty()) {
					m["INT"]	= TYPE::INT;
					m["UINT"]	= TYPE::UINT;
					m["STRING"] = TYPE::STRING;
					m["INT64"]	= TYPE::INT64;
				}

				return m;
			}

			static size_t width_for_type(const TYPE t)
			{
				switch (t) {
					case INT :
						return std::numeric_limits<int32_t>::digits10 + 1;

					case UINT:
						return std::numeric_limits<uint32_t>::digits10;

					case STRING:
						return DEFAULT_STRING_WIDTH;

					case INT64:
						return std::numeric_limits<int64_t>::digits10 + 1;

					default:
						THROW_ERR(-1, "width_for_type: unknown type");
				};

				// return 0;
			}

			static TYPE type_from_name(const std::string& name)
			{
				static const map_t& m = type_map();
				TYPE ret = TYPE::INT;
				auto it = m.find(name);

				if (it == m.end()) {
					THROW_ERR( -EINVAL, "column::TYPE() column id ",
							   name , " is not found");
				} else {
					ret = it->second;
				}

				return ret;
			}

			static std::string type_to_name(const TYPE t)
			{
				static const map_t& m = type_map();

				for (auto& pr : m) {
					if (pr.second == t) {
						return pr.first;
					}
				}

				THROW_ERR(-EEXIST, "column_type::type_to_name(): no match for type", static_cast<int64_t>(t));
				
			}
		}; // struct column_types

	}// namespace db
} // namespace cpp

#endif // DBYPES_H
