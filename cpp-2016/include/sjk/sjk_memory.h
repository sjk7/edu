#pragma once
#ifndef SJK_MEMORY_H
#define SJK_MEMORY_H
#include <algorithm>
#include <cassert>
#include <cstring> // memcpy
#include "sjk_exception.h"
#include <string>
#include <vector>
#include <type_traits>

namespace sjk {

    using byte = char; 
    template <typename T> struct not_null {
        not_null(T *p) noexcept : m_p(p) { assert(m_p); }
        constexpr operator const T *() const noexcept { return m_p; }
        T *operator->() noexcept { return m_p; }

        private:
        T *const m_p;
    }; // not_null <T>

    template <typename T, size_t n>
    constexpr size_t array_size(const T (&)[n]) noexcept {
        return n;
    }

#ifdef _MSC_VER
#pragma warning (disable: 26481)
#pragma warning (disable: 26493)
#endif
    namespace detail {
        template <typename T> struct ptrs {

            using ctc = const T *const;
            using tc = const T *;
            explicit ptrs(ctc beg = nullptr, ctc end = nullptr,
                          tc d = nullptr) noexcept : m_begin((T *)beg),
                m_end((T *)end),
                m_data((T *)d) {
                if (m_begin && !m_data)
                    m_data = m_begin;
                sanity();
            }
            // NOTE here: count, not size: this is not a BYTE count,
            // its a count of Ts
            constexpr explicit ptrs(ctc beg, size_t count) noexcept
                : m_begin((T *)beg),
                  m_end(m_begin + count),
                  m_data((T *)beg) {}
            T *m_begin;
            T *m_end;
            T *m_data;

            void sanity() noexcept {
                if (m_begin || m_data || m_end) {
                    assert(m_data && m_begin && m_end);
                    assert(m_end >= m_begin);
                    assert(m_data >= m_begin && m_data <= m_end);
                }
            }
        };

    } // namespace detail

    template <typename T> struct span;
    template <class X> struct has_cont_storage : public std::false_type {};
    template <class X, class A>
    struct has_cont_storage<std::vector<X, A>> : public std::true_type {};
    template <> struct has_cont_storage<std::string> : public std::true_type {};

    template <typename B> struct is_span : public std::false_type {};
    template <class Y> struct is_span<span<Y>> : public std::true_type {};

    template <typename A, typename B, typename C = A>
    struct is_bigger : public std::false_type {};
    template <typename A, typename B>
    struct is_bigger<A, B, typename std::enable_if<(sizeof(A) > sizeof(B))>::type>
        : public std::true_type {
                     typedef A type;
};
template <typename A, typename B>
struct is_bigger<A, B, typename std::enable_if<(sizeof(A) < sizeof(B))>::type>
    : public std::false_type {
                 typedef B type;
};

template <typename T> struct span {
    public:
    using ctc = const T *const;
    using tc = const T *;
    using ptrs = detail::ptrs<T>;
    typedef T span_type;

    protected:
    ptrs m_p;
	int64_t m_datalen{ 0 }; // socket uses this to let caller know how much data

    public:
    constexpr span() noexcept {}
    span(detail::ptrs<byte> &p) noexcept : m_p(p) {

    }

	void datalen_set(const int64_t newsize) {
		assert(newsize <= (int64_t)size_bytes());
		m_datalen = newsize;
	}

	int64_t datalen() const {
		return m_datalen;
	}

    /*/ //DISABLED: use span_t(&myarray[0], array_size), as it expresses intentions better (and leads to tons of warnings from the gsl, if enabled.
  template <typename Q, size_t N> span(const Q (&buf)[N]) noexcept {
        ptrs pd(buf, buf + N, buf);
        m_p = pd;
  }
  /*/
    span(T* p, const size_t N) noexcept {
        ptrs pd(p, p + N, p);
        m_p = pd;
    }

    template <typename W, typename U = W>
	span(W &t, typename std::enable_if<std::is_standard_layout<U>::type>::value * = 0,
		 typename std::enable_if<std::is_same<typename std::remove_reference<U>::type::value,
         typename std::remove_reference<span_type>::type>::value> * = 0) noexcept {
        ptrs pd(&t, 1);
        m_p = pd;
    }
#ifdef _MSC_VER
#pragma warning(disable: 26490)
#endif

	template <typename RB>
	span(const RB* t, const size_t sz)
	{
		m_p.m_begin = (span_type*)t;
		m_p.m_end = m_p.m_begin + sz;
		m_p.m_data = m_p.m_begin;
	}


    span(span<T>& other) noexcept : m_p(other.m_p)  {}
    span(span<T>&& other) noexcept : m_p(other.m_p)  {}


    template <typename W, typename U = W>
	span(W &t, typename std::enable_if<std::is_standard_layout<U>::type>::value * = 0,
		 typename std::enable_if<!std::is_same<typename std::remove_reference<U>::type,
         typename std::remove_reference<span_type>::type>::type>::value * = 0,
		 typename std::enable_if<!has_cont_storage<U>::type>::value * = 0) noexcept {
        // the type of the span and what we are sending is not the same,
        // so the sensible thing to do is convert to bytes.
        // Here, we assert for consistency between the types:
        static_assert(sizeof(span_type) == 1, "expected to be counting in bytes here");
        ptrs pd(reinterpret_cast<span_type*>(&t), sizeof(t));
        m_p = pd;

    }

	span(const char* s) 
	{
		auto sz = strlen(s);
		m_p.m_begin = s;
		m_p.m_data = m_p.m_begin;
		m_p.m_end = m_p.m_begin + sz;
	}
    
    span(const std::string& s) :
        m_p(s.data(), s.data()+ s.size(), s.data())
    {

    }

    template <typename C, typename VS = C>
    span(const C &c,
		 typename std::enable_if<has_cont_storage<VS>::type>::type::value * = 0) noexcept {
        ptrs pd(c.data(), c.data() + c.size(), c.data());
        m_p = pd;
    }


    const span &operator=(const span &rhs) noexcept {
        m_p = rhs.m_p;
        return *this;
    }


    template <typename R> span &operator=(const R &rhs) noexcept {
        assign(rhs);
        return *this;
    }

    size_t size() const noexcept {
        size_t sz = (size_t)(m_p.m_end - m_p.m_begin);
        return sz;
    }
    size_t size_bytes() const noexcept {
        size_t ret = size();
        ret *= sizeof(T);
        return ret;
    }

#ifdef _WIN32 // DWORD
    unsigned long size_bytes_d() const noexcept
    {
        return static_cast<unsigned long>(size());
    }
#endif
    T *begin() const noexcept { return m_p.m_begin; }
    T *end() const noexcept { return m_p.m_end; }
    T *data() noexcept { return m_p.m_data; }
    void sanity() noexcept { m_p.sanity(); }
    constexpr bool empty() const noexcept { return size() == 0; }

    template <typename R, typename D = R>
    span &assign(
			const R &rhs, typename std::enable_if<is_span<D>::value> * = 0,
			typename std::enable_if<
            is_bigger<typename std::remove_reference<typename D::span_type>::type,
            span_type>::value> * = 0) noexcept {
        rhs.copy(*this);
        return *this;
    }

    template <typename R, typename D = R>
#ifndef SPAN_NARROWING_WARNING_SUPPRESS
    [[deprecated("This assignment causes a narrowing conversion: possible loss "
                 "of data...\n #define SPAN_NARROWING_WARNING_SUPPRESS to "
                 "disablethis warning")]]
#endif
    span &
	assign(const R &rhs, typename std::enable_if<is_span<D>::value> * = 0,
		   typename std::enable_if<!is_bigger<
           typename std::remove_reference<typename D::span_type>::type,
           span_type>::value> * = 0) noexcept {
        rhs.copy(*this);
        return *this;
    }

    template <typename R, typename D = R>
    span &
	assign(const R &rhs, typename std::enable_if<!is_span<D>::value> * = 0,
		   typename std::enable_if<!is_bigger<typename std::remove_reference<D>::type,
           span_type>::value> * = 0) noexcept {
        ptrs pd(&rhs, 1);
        m_p = pd;
        return *this;
    }

    operator span<byte>() noexcept {
        detail::ptrs<byte> ptrs((byte *)begin(), size_bytes());
        span<byte> ret(ptrs);
        return ret;
    }

    void ptrs_set(ptrs p) {
        m_p = p;
        sanity();
    }

    template <typename OTHERTYPE> operator span<OTHERTYPE>() const {
        span<OTHERTYPE> ret;
        detail::ptrs<OTHERTYPE> pts((OTHERTYPE *)m_p.m_begin,
                                    (OTHERTYPE *)m_p.m_end);
        ret.ptrs_set(pts); // copy won't do anything if we don't do this first.
        copy(ret);
        return ret;
    }

    template <typename C,
			  typename D = typename std::enable_if<has_cont_storage<C>::value>>
    const span &operator=(const C &c) noexcept {
        ptrs pd(c.data(), c.data() + c.size(), c.data());
        m_p = pd;
    }

#ifdef SPAN_NOEXCEPT_GET_REF
    operator T &() noexcept {
        static T t;
        if (!m_p.m_begin)
            return t;
        return *m_p.m_begin;
    }
#else
    operator T &() {
		if (!m_p.m_begin) {
			SJK_EXCEPTION("sjk::span: Tried to dereference an empty span");
		}
        return *m_p.m_begin;
    }
#endif

    T &operator[](size_t i) {
#ifndef NDEBUG
		const size_t sz = size();
		if (i >= sz) {
			SJK_EXCEPTION("sjk::span: operator []");
		}
#endif
        return *(m_p.m_begin + i);
    }



    template <typename SPAN, typename S = SPAN>
    size_t
    copy(SPAN &dest,
		 typename std::enable_if<std::is_same<typename S::span_type, span_type>::value::type>
         * = 0) const noexcept {
        if (&dest == this)
            return 0;
        assert(!empty()); // resize your storage first
        if (dest.empty() || empty())
            return 0;
        size_t sz = std::min(size(), dest.size());
        std::copy(begin(), begin() + sz, dest.data());
        return sz;
    }


    /*!
     * \brief copy into me from a std::string
     * \param s
     * \return the number of *bytes* copied (which includes the null terminator)
     */
    size_t copy(const std::string& sfrom)  noexcept
    {
		span<const char> spsrc(sfrom.data(), sfrom.size());
		return copy_string_internal(spsrc, *this);
    }



	private:

	// copy FROM me into spdest. Null term is always appended,
	// and I return the total number of BYTES copied, including the null term.
	// If dest is not null, and we have nothing to copy, we zero out dest_size bytes in dest.
	// It's allowed to modify itself, so it can't be const
	size_t copy_string_internal(span<const char> spsrc, span<char> spdest) noexcept
	{
		size_t szfrom = spsrc.size_bytes();
		size_t szto = spdest.size_bytes();
		char* dest = spdest.data();
		const char* src = spsrc.data();

		const char* dend = dest + szto; (void)dend;

		assert(dest); if (!dest) return 0;
		if (szfrom == 0) {
			memset(dest, 0, szto);
			return 0;
		}
		size_t sz = std::min(szfrom, szto - 1);
		if (sz >= szto) {
			sz--; //corner case. Paranoia. It may only ever be *equal* to
		}
		if (sz == 0) return 0;
		assert(begin());
		memcpy(dest, src, sz);

		// here I append the null
		char* plast = dest + sz; // correct, it goes AFTER the stuff we copied.
		assert(plast < dend);
			
		*plast = '\0';
		return sz + 1; // return THE NUMBER OF BYTES copied

	}
	public:
	// copy FROM me into spdest. Null term is always appended,
	// and I return the total number of BYTES copied, including the null term.
	size_t copy_string(span<char>spdest)  noexcept
	{
		return copy_string_internal(*this, spdest);
	}
	


template <typename SPAN, typename S = SPAN>
size_t
copy(SPAN &dest,
	 typename std::enable_if<!std::is_same<typename S::span_type, span_type>::value>::type * = 0)
	 const noexcept 

{
    // for differing types of span, we can do a straight memcpy
    //, if constructors don't need to be called
    static_assert(std::is_standard_layout<span_type>::value &&
                  std::is_default_constructible<span_type>::value,
                  "When copying spans of differing type, both spans must have "
                  "standard layout.");
    static_assert(
                std::is_standard_layout<typename S::span_type>::value &&
                std::is_default_constructible<typename S::span_type>::value,
                "When copying spans of differing type, both spans must have standard "
                "layout.");
    size_t sz1 = size_bytes();
    size_t sz2 = dest.size_bytes();
    if (dest.empty() || empty())
        return 0;

    size_t sz = std::min(sz1, sz2);
    assert(dest.data());
    assert(begin());
    memcpy((byte *)dest.data(), (byte *)begin(), sz);
    // assert("constructors will not be called for a variant of this type" ==
    // 0);

    return sz;
}
};

} // namespace sjk

#ifdef _MSC_VER
#pragma warning (default: 26481)
#pragma warning (default: 26493)
#endif
#endif
