#pragma once
#ifndef SJK_RECORD_SET_H
#define SJK_RECORD_SET_H
#include "sjk_db.h"

namespace sjk
{
	namespace db
	{
		template <typename R>
		struct icolumn
		{
			typedef typename sjk::db::row_type row_t;
			virtual void populate(R& r) = 0;
			virtual std::string value_string(const row_t) const = 0;
			virtual void reserve(const size_t hint) = 0;
			virtual sjk::db::row_t row_count() const = 0;
			virtual ~icolumn(){}
		};

		template <typename T, typename R, typename POP>
		struct column : public icolumn<R>
		{
			using vt_t = std::vector<T>;

			column(const std::string&  name) : m_sname(name){}
			const std::string& name() const{ return m_sname;}
			sjk::db::row_t row_count() const override{
				return sjk::db::row_t(m_vec_values.size());
			}
			std::string value_string(const row_t row) const override
			{
				return value(row); // stringstream is *awful* slow (100x!)
			}

			T& value(const row_t row)const{
				return m_vec_values.at(row.value());
			}
			template<typename cmp = std::less<T> >
			void sort(cmp = std::less<T>()){
				std::stable_sort(m_vec_values.begin(), m_vec_values.end(), cmp());
			}

		protected:
			virtual void reserve(const size_t hint) override{
				m_vec_values.reserve(hint);
			}

			std::string m_sname;
			mutable vt_t m_vec_values;
		};

		template <typename DB>
		struct record_set : public sjk::db::notifier<typename DB::record_type>
		{
			using R = typename DB::record_type;
			using columns_t = std::vector<icolumn<R>*>;

			record_set (DB& db) : m_db(db){
				m_db.dispatch_manager().notifier_add(this);
			}
			sjk::db::row_t row_count() const {
				if (m_columns.empty()) return sjk::db::row_t(0);
				return sjk::db::row_t(m_columns.at(0)->row_count());
			}
			template <typename POP>
			void column_add(POP& p)
			{
				m_columns.push_back(&p);
			}
			const columns_t& columns() const{
				return m_columns;
			}

		protected:
			DB& m_db;
			columns_t m_columns;

			virtual void prepopulate(const size_t hint) override{
				for (auto col : m_columns){
					col->reserve(hint);
				}
			}

			virtual int on_record_populate(R& r) override {
				for (auto col : m_columns){
					col->populate(r);
				}
				return 0;
			}
		};

		template <typename T, typename R, typename P>
		struct populator : column<T, R, P>
		{
			using col_t = column<T, R, P>;
			populator (const std::string& name) :
			col_t(name){}
			virtual ~populator(){}
		};

	} // namespace db

} // namespace sjk

#endif // SJK_RECORD_SET_H
