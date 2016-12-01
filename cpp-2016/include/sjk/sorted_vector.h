#pragma once
//-------------------------------------------------------------------//
//	Copyright © 2001-2011 A better Software.
//
// Use at your own risk.  :>
//
// History
// -------
// v0.1 1999? A long time ago in a compiler far far away...
// .
// v0.9 12/1/2010   Added predicate function support
// v1.0 1/23/2010   Class first archived and documented on wiki
//                   https://www.thedigitalmachine.com/wiki/index.php/C%2B%2B_sorted_vector
// v1.1 1/24/2010   Fixed predicate version of find_it_or_fail to actually use predicate
// v1.2 2/04/2011   Added find_ptr_or_fail
//-------------------------------------------------------------------//

#if !defined(SORTED_VECTOR_H)
#define SORTED_VECTOR_H

#include <vector>
#include <algorithm>			// For lower_bound


//-------------------------------------------------------------------//
// sorted_vector																		//
//-------------------------------------------------------------------//
// This class is derived from std::vector, and adds sort tracking.
// There are two basic ways to use a sorted vector:
//
//		METHOD 1
//			Always maintain sort order by inserting with push_sorted() -
//			the location of new items is determined before inserting;
//			since the vector remains sorted, this doesn't take too long
//			(although for large batch insertions METHOD 2 is definitely
//			faster);
//
//		METHOD 2
//			Allow batch insertion without sorting with push_unsorted(); then
//			provide an additional call to sort the vector;  before
//			searching for an item, the vector is always sorted if needed;
//
//  Of course you need to provide an operator()< for the type of object
//  you're sorting, if it doesn't have one.  Example:
	/*
		class MyClass
		{
		public:
			bool operator< (const MyClass left) const
			{
				if ( left.m_nMostImportant == m_nMostImportant )
					return left.m_nLeastImportant < m_nLeastImportant;

				return left.m_nMostImportant < m_nMostImportant;
			}

			int m_nMostImportant;
			int m_nLeastImportant;
		}
	*/
//
//  NOTE: C++ doesn't let you use an operator()< for POINTERS.  This
//  breaks down when creating the template code, as you end up with
//  a ref to a ref which is not allowed (or something :P).
//  So if you have a vector of pointers, here's what you have to do:
//
//      FOR NOW, with straight C++, create a less-than functor, then
//      pass that in to the functor versions of the class methods below.
//      Create a functor, aka function object, as follows:
//
//          struct my_class_lessthan
//          {
//              bool operator()(const MyClass* left, const MyClass* right)
//              {
//                  return left->get_timestamp() < right->get_timestamp();
//              }
//          };
//
//      Usage example:
//
//          sorted_vector<MyClass*> svpMC;
//          svpMC.push_unsorted(new MyClass(blah, blah);
//          svpMC.push_unsorted(new MyClass(blah, blah);
//          vpMC.sort( my_class_lessthan() );
//
//      Once C++0x is available, I need to update this class to use
//      a function object wrapper, and allow the user to set it
//      in the constructor, then always use it automatically.
//      http://en.wikipedia.org/wiki/C%2B%2B0x#Polymorphic_wrappers_for_function_objects
//
// WARNING: if you change the key value of any object in the vector,
// you have unsorted the array without marking it as such.  Make sure
// you call SetSorted(false) where appropriate.
//
//-------------------------------------------------------------------//
template<class T, class _A = std::allocator<T> >
class sorted_vector : public std::vector<T, _A>
{
	typedef std::vector<T> inherited;

public:
	using allocator_type = typename inherited::allocator_type;
	using value_type = typename inherited::value_type;

    sorted_vector(std::initializer_list<value_type> l,
                  allocator_type a = typename inherited::allocator_type()) :
        inherited(l, a){
        sort();
    }
    sorted_vector(const sorted_vector& other,
                  allocator_type a = typename inherited::allocator_type()) : inherited(other, a),
        m_bSorted(other.m_bSorted){

    }

    sorted_vector(sorted_vector&& other,
                  allocator_type a = typename inherited::allocator_type()) :
        inherited(other, a)
    {
        m_bSorted = other.m_bSorted;
    }
    sorted_vector() :
        m_bSorted(false){}

	//-------------------------------------------------------------------//
	// SetSorted()																			//
	//-------------------------------------------------------------------//
	// I didn't want to override every constructor to set this
	// member variable, so this function is publicly accessible.
	// You should call SetSorted( true ) right after construction.
	// TO DO: if you feel like it... derive all constructors to avoid
	// the need for this.  There are 4 last time I checked.
	//-------------------------------------------------------------------//
	private:
	void SetSorted( bool bSorted = true ) { m_bSorted = bSorted; }
	public:
	// ^^ MOVED to private by steve due to brace initialization in c++11 +
	//-------------------------------------------------------------------//
	// sort()                                                            //
	//-------------------------------------------------------------------//
	// This function sorts the data as needed.  Call it after repeated calls to
	// push_unsorted(), or just let other members call it for you on next access.
	// It calls std::sort(), which defaults to using operator<() for
	// comparisons.
	//-------------------------------------------------------------------//
	void sort()
	{
		if ( !m_bSorted )
		{
			std::sort( inherited::begin(), inherited::end() );
			SetSorted();
		}
	}

	// This function is stupid.  binary_search() was a dumb design, STL peeps.
	bool bContains( const T& t )
	{
		if ( !m_bSorted )
		{
			sort();
		}

		return std::binary_search( inherited::begin(), inherited::end(), t );
	}

	typename std::vector<T>::iterator lower_bound_it( const T& key )
	{
		if ( !m_bSorted )
		  sort();

		typename std::vector<T>::iterator it = std::lower_bound( inherited::begin(), inherited::end(), key );
		return it;
	}

	/*const*/ T* lower_bound_ptr( const T& key )
	{
		typename std::vector<T>::iterator it = lower_bound_it( key );

		if (it==inherited::end())
			return 0;

		/*const*/ T* t = &(*it);
		return t;
	}


	//-------------------------------------------------------------------//
	// find_it_or_fail()                                                 //
	//-------------------------------------------------------------------//
	// This function takes the given object and determines if there is
	// a match in the vector.  It returns an iterator to the actual
	// object in the vector, if found.  Otherwise returns std::vector::end().
	//
	// This is the function you want to use most of the time
	// (or the predicate version if you are using object pointers).
	//
	// USAGE: it makes most sense to use this function if you have
	// an object with a key, other member variables, and operator<()
	// that uses the key to test for equality.  You then set up a dummy
	// "search" object with the key set to the search value, call the
	// function, and use the result to extract the additional information
	// from the object.
	//-------------------------------------------------------------------//
	 typename std::vector<T>::iterator find_it_or_fail( const T& key )
	 {
		 typename std::vector<T>::iterator it = lower_bound_it( key );

		  if ( it != inherited::end() )

			  // lower_bound() does not necessarily indicate a successful search.
			  // The iterator points to the object where an insertion
			  // should take place.  We check that result to see if we actually
			  // had an exact match.

			  // NOTE: This is how the STL determines equality using only operator()<.
			  // Two comparisons, ugg, but it is a nice little trick.
			  if( !((*it)<key) && !(key<(*it)) )

				  return it;

		  return inherited::end();
	 }

	typename std::vector<T>::iterator find( const T& key ){
		return find_it_or_fail(key);
	}

    bool erase(typename inherited::iterator it){
		return (inherited::erase(it) != inherited::end());
	}

    bool erase(const T& key){
        bool ret = false;
        auto it = find_it_or_fail(key);
        if (it != this->cend()){
            ret = true;
            inherited::erase(it);
        }

        return ret;
    }

	 //-------------------------------------------------------------------//
	 // find_ptr_or_fail()                                                 //
	 //-------------------------------------------------------------------//
	 // A variation of find_it_or_fail() that provides a pointer to result.
	 //-------------------------------------------------------------------//
	  T* find_ptr_or_fail( const T& key )
	  {
		  typename std::vector<T>::iterator it = find_it_or_fail( key );
		  if ( it != inherited::end() )
			  return &(*it);

		  return 0;
	  }

	//-------------------------------------------------------------------//
	// push_sorted()																		//
	//-------------------------------------------------------------------//
	// This is used to insert into a vector that always remains sorted.
	// Because we have a sorted vector, finding the insertion location
	// with std::lower_bound() is relatively cheap.
	//
	// If you have multiple insertions, consider
	// using push_unsorted() for each, then calling sort().
	//-------------------------------------------------------------------//
	typename std::vector<T>::iterator  push_sorted( const T& t )
	{
		if ( !m_bSorted )
		{
			sort();
		}

		// Insert at "lower_bound" (the proper sorted location).
		return std::vector<T>::insert( std::lower_bound( inherited::begin(), inherited::end(), t ), t );
	}

	  /*!
	 * \brief insert : returns false if it was already present, and does not do anything else.
	 *
	 * \param t
	 * \return Specifically *not* an iterator, because if you have if (insert(x) == vec::end())
	 * then you fall into the order of evaluation UB. (which happens for me in gcc only: nasty!_
	 */
	bool insert( const T& t)
	{
		if (find(t) == std::vector<T>::end()){
			push_sorted(t);
			return true;
		}
		return false;
	}

	//-------------------------------------------------------------------//
	// push_unsorted()																	//
	//-------------------------------------------------------------------//
	// This is similar to push_back(), but in addition, it sets the
	// unsorted flag.
	//-------------------------------------------------------------------//
	void push_unsorted( const T& t )
	{
		SetSorted( false );
		push_back(t);
	}

	//-------------------------------------------------------------------//
	// operator=()																	//
	//-------------------------------------------------------------------//
	// This allows us to set the sorted_vector from a std::vector.
	//-------------------------------------------------------------------//
	sorted_vector<T>& operator=(std::vector<T>& v)
	{
		typename std::vector<T>::iterator it;
		for (it= v.begin(); it != v.end(); ++it)
			push_unsorted((*it));
		return this;
	}

	// CALLS WHERE YOU PROVIDE THE FUNCTOR OR FUNCTION POINTER
	// If you need to use a predicate sort function, ALWAYS use these methods
	// instead of the non-functor versions.
	// NOTE: UPDATE THIS when C++0x polymorphic function wrappers are available.
   template<class _Pr> inline
	void sort( _Pr pr )
	{
		if ( !m_bSorted )
		{
				  std::sort( inherited::begin(), inherited::end(), pr );
			SetSorted();
		}
	}
	template<class _Pr> inline
	 typename std::vector<T>::iterator lower_bound_it( const T& key, _Pr pr )
	 {
		 if ( !m_bSorted )
		 {
			 std::sort( inherited::begin(), inherited::end(), pr );
			 SetSorted();
		 }
		 typename std::vector<T>::iterator it = std::lower_bound( inherited::begin(), inherited::end(), key, pr );
		 return it;
	 }
	template<class _Pr> inline
	/*const*/ T* lower_bound_ptr( const T& key, _Pr pr )
	{
		typename std::vector<T>::iterator it = lower_bound_it( key, pr );

		if (it==inherited::end())
			return 0;

		/*const*/ T* t = &(*it);
		return t;
	}
	 template<class _Pr> inline
	 void push_sorted( const T& t, _Pr pr )
	 {
		 if ( !m_bSorted )
		 {
			 std::sort( inherited::begin(), inherited::end(), pr );
			 SetSorted();
		 }

		 // Insert at "lower_bound" (the proper sorted location).
		 insert( std::lower_bound( inherited::begin(), inherited::end(), t, pr ), t );
	 }
	 template<class _Pr> inline
	  typename std::vector<T>::iterator find_it_or_fail( const T& key, _Pr pr )
	  {
		  typename std::vector<T>::iterator it = lower_bound_it( key, pr );

		  if ( it != inherited::end() )

			  // NOTE: We have to apply this using the predicate function, be careful...
			  if (!(pr((*it), key)) && !(pr(key,(*it))))

				  return it;

		  return inherited::end();
	  }
	  template<class _Pr> inline
	  T* find_ptr_or_fail( const T& key, _Pr pr )
	  {
		  typename std::vector<T>::iterator it = find_it_or_fail( key, pr );
		  if ( it != inherited::end() )
			  return &(*it);

		  return 0;
	  }

protected:
	bool m_bSorted{};

};

#endif // __SORTED_VECTOR_H

