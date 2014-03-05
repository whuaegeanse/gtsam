/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/*
 * @file DerivedValue.h
 * @date Jan 26, 2012
 * @author Duy Nguyen Ta
 */

#pragma once
#include <boost/make_shared.hpp>
#include <gtsam/global_includes.h>
#include <gtsam/base/Value.h>

//////////////////
// The following includes windows.h in some MSVC versions, so we undef min, max, and ERROR
#include <boost/pool/singleton_pool.hpp>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef ERROR
#undef ERROR
#endif
//////////////////


namespace gtsam {

template<class DERIVED>
class DerivedValue : public Value {

protected:
  DerivedValue() {}

public:

  virtual ~DerivedValue() {}

  /**
   * Create a duplicate object returned as a pointer to the generic Value interface.
   * For the sake of performance, this function use singleton pool allocator instead of the normal heap allocator.
   * The result must be deleted with Value::deallocate_, not with the 'delete' operator.
   */
  virtual Value* clone_() const {
#ifdef GTSAM_ALLOCATOR_TBB
    void *place = tbb::tbb_allocator<DERIVED>().allocate(1);
#else
    void *place = boost::singleton_pool<PoolTag, sizeof(DERIVED)>::malloc();
#endif
    DERIVED* ptr = new(place) DERIVED(static_cast<const DERIVED&>(*this));
    return ptr;
  }

  /**
   * Destroy and deallocate this object, only if it was originally allocated using clone_().
   */
  virtual void deallocate_() const {
    this->~DerivedValue(); // Virtual destructor cleans up the derived object
#ifdef GTSAM_ALLOCATOR_TBB
    tbb::tbb_allocator<DERIVED>().deallocate((DERIVED*)this, 1);
#else
    boost::singleton_pool<PoolTag, sizeof(DERIVED)>::free((void*)this); // Release memory from pool
#endif
  }

  /**
   * Clone this value (normal clone on the heap, delete with 'delete' operator)
   */
  virtual boost::shared_ptr<Value> clone() const {
    return boost::make_shared<DERIVED>(static_cast<const DERIVED&>(*this));
  }

  /// equals implementing generic Value interface
  virtual bool equals_(const Value& p, double tol = 1e-9) const {
    // Cast the base class Value pointer to a derived class pointer
    const DERIVED& derivedValue2 = dynamic_cast<const DERIVED&>(p);

    // Return the result of calling equals on the derived class
    return (static_cast<const DERIVED*>(this))->equals(derivedValue2, tol);
  }

  /// Generic Value interface version of retract
  virtual Value* retract_(const Vector& delta) const {
    // Call retract on the derived class
    const DERIVED retractResult = (static_cast<const DERIVED*>(this))->retract(delta);

    // Create a Value pointer copy of the result
#ifdef GTSAM_ALLOCATOR_TBB
    void *resultAsValuePlace = tbb::tbb_allocator<DERIVED>().allocate(1);
#else
    void *resultAsValuePlace = boost::singleton_pool<PoolTag, sizeof(DERIVED)>::malloc();
#endif
    Value* resultAsValue = new(resultAsValuePlace) DERIVED(retractResult);

    // Return the pointer to the Value base class
    return resultAsValue;
  }

  /// Generic Value interface version of localCoordinates
  virtual Vector localCoordinates_(const Value& value2) const {
    // Cast the base class Value pointer to a derived class pointer
    const DERIVED& derivedValue2 = dynamic_cast<const DERIVED&>(value2);

    // Return the result of calling localCoordinates on the derived class
    return (static_cast<const DERIVED*>(this))->localCoordinates(derivedValue2);
  }

  /// Assignment operator
  virtual Value& operator=(const Value& rhs) {
    // Cast the base class Value pointer to a derived class pointer
    const DERIVED& derivedRhs = dynamic_cast<const DERIVED&>(rhs);

    // Do the assignment and return the result
    return (static_cast<DERIVED*>(this))->operator=(derivedRhs);
  }

  /// Conversion to the derived class
  operator const DERIVED& () const {
    return static_cast<const DERIVED&>(*this);
  }

  /// Conversion to the derived class
  operator DERIVED& () {
    return static_cast<DERIVED&>(*this);
  }

protected:
  /// Assignment operator, protected because only the Value or DERIVED
  /// assignment operators should be used.
  DerivedValue<DERIVED>& operator=(const DerivedValue<DERIVED>& rhs) {
    // Nothing to do, do not call base class assignment operator
    return *this;
  }

private:
  /// Fake Tag struct for singleton pool allocator. In fact, it is never used!
  struct PoolTag { };

};

} /* namespace gtsam */
