#ifndef TEXT_IMPORTER_H
#define TEXT_IMPORTER_H

#include "strings.h"
#include "collection.h"
#include "utils.h" // filesize()


namespace cpp
{


	class import_col
	{
		public:
		using data_t = std::vector<std::string>;
		import_col(const std::string name) : m_name(name){}
		import_col(){}
		data_t m_data;
		std::string m_name;
	};

	class text_importer
	{
		public:

		typedef cpp::collections::collection<std::string, import_col> cols_t;
		cols_t m_col;
		text_importer(const std::string& path, const std::string delim = "\t") : m_delim(delim), m_path(path)
		{
			parse();
		}

		int64_t rowcount() const{
			if (m_col.empty()) return 0;
			const auto& c = m_col[0];
			return static_cast<int64_t>(c.m_data.size());
		}

		private:
		void parse(){

			std::fstream f;
			f.exceptions(std::ios_base::badbit | std::ios_base::failbit);
			f.open(m_path.c_str(), std::ios_base::binary | std::ios_base::in);
			auto fsize = static_cast<size_t>(cpp::filesize(f));
			std::string s(fsize,'\0');
			f.read(&s[0], fsize);

			cpp::strings::strvec_t v;
			cpp::strings::split(s, "\r\n", v);
			s.clear(); s.shrink_to_fit();

			const std::string TAB("\t");

			const auto& first_line = v.at(0);
			cpp::strings::strvec_t vline;
			cpp::strings::split(first_line, TAB, vline);

			for (const auto& sname : vline){
				m_col.add(sname, import_col(sname));
			}

			int ctr =0; 
			for (auto& col : m_col){
				col->m_data.resize(v.size());
			}
			for (const auto& sline : v ){
				
				if (ctr > 0){
					
					cpp::strings::split(sline, TAB, vline);
					THROW_ASSERT(vline.size() == m_col.size(), -1, 
						"text_importer: bad line size. Wanted" , m_col.size(), " but got", vline.size());
					int colctr = 0;
					for (auto& column: m_col)
					{
						const auto& sdata = vline[colctr];
						column->m_data[ctr-1] = sdata;
						colctr++;
					}
				}
				

				ctr++;
			}
		}

		std::string m_delim;
		std::string m_path;

	};
}

#endif // TEXT_IMPORTER_H
