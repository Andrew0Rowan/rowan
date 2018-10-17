
#ifndef _SMARTAUDIOPLAYER_REFPTR_H
#define _SMARTAUDIOPLAYER_REFPTR_H


/// @brief Reference counted pointer
template <typename X> class RefPtr
{
public:
    /// @brief Element type
    typedef X element_type;

    /// @brief Friend refptr class
    template <class Y> friend class RefPtr;

    /// @brief Constructor
    RefPtr() :
        m_RawPtr(0),
        m_Counter(0)
    {
    }

    /// @brief Constructor
    RefPtr(X* raw) :
        m_RawPtr(0),
        m_Counter(0)
    {
        if (raw)
        {
            m_RawPtr = raw;
            m_Counter = new M_Counter;
        }
    }

    /// @brief Constructor
    template <typename Y> RefPtr(Y* raw) :
        m_RawPtr(0),
        m_Counter(0)
    {
        if (raw)
        {
            m_RawPtr = static_cast<X*>(raw);
            m_Counter = new M_Counter;
        }
    }

    /// @brief Constructor
    RefPtr(const RefPtr<X>& otherPtr):
        m_RawPtr(0),
        m_Counter(0)

    {
        Acquire(otherPtr.m_Counter);
        m_RawPtr = otherPtr.m_RawPtr;
    }

    /// @brief Constructor
    template <typename Y> RefPtr(const RefPtr<Y>& otherPtr) :
        m_RawPtr(0),
        m_Counter(0)
    {
        Acquire((M_Counter *)(otherPtr.m_Counter));
        m_RawPtr = static_cast<X*>(otherPtr.Get());
    }


    /// @brief Destructor
    ~RefPtr()
    {
        Release();
    }

    /// @brief Operator =
    RefPtr& operator=(const RefPtr<X>& otherPtr)
    {
        if (this != &otherPtr)
        {
            if(m_RawPtr != otherPtr.m_RawPtr)
            {
                Release();
                Acquire(otherPtr.m_Counter);
                m_RawPtr = otherPtr.m_RawPtr;
            }
        }
        return *this;
    }

    /// @brief Operator =
    template <typename Y> RefPtr& operator=(const RefPtr<Y>& otherPtr)
    {
        if (this != (RefPtr<X> *)&otherPtr)
        {
            if(m_RawPtr != static_cast<X*> (otherPtr.Get()))
            {
                Release();
                Acquire((M_Counter *)(otherPtr.m_Counter));
                m_RawPtr = static_cast<X*> (otherPtr.Get());
            }
        }
        return *this;
    }

    /// @brief Operator =
    RefPtr& operator=(X* raw)
    {
        if(m_RawPtr != raw)
        {
            Release();
            if (raw)
            {
                m_Counter = new M_Counter;
                m_RawPtr = raw;
            }
        }

        return *this;
    }

    /// @brief Operator =
    template <typename Y> RefPtr& operator=(Y* raw)
    {
        if(m_RawPtr != static_cast<X*>(raw))
        {
            Release();
            if (raw)
            {
                m_Counter = new M_Counter;
                m_RawPtr = static_cast<X*>(raw);
            }
        }
        return *this;
    }

    /// @brief Operator ->
    X* operator->() const
    {
        return Get();
    }

    /// @brief Operator *
    X& operator *() const
    {
        return *Get();
    }

    /// @brief Cast to bool
    operator bool() const
    {
        return IsValid();
    }

    /// @brief Cast to pointer type
    template <typename Y> operator Y *() const
    {
        return static_cast<Y*>(m_RawPtr);
    }

    /// @brief Cast to pointer type
    template <typename Y> operator const Y *() const
    {
        return static_cast<const Y*>(m_RawPtr);
    }

    /// @brief Returns the underlying pointer
    X* Get() const
    {
        return m_RawPtr;
    }

    /// @brief Is this a 0 pointer?
    bool Null() const
    {
        return m_RawPtr == 0;
    }

    /// @brief Is the pointer valid?
    bool IsValid() const
    {
        return (/*m_Counter &&*/ m_RawPtr != 0);
    }

    /// @brief Returns the value of the reference counter
    int GetCount() const
    {
        if (m_Counter)
        {
            return m_Counter->m_Count;
        }
        else
        {
            return 0;
        }
    }

private:
    X* m_RawPtr;

    struct M_Counter
    {
        //explicit M_Counter(unsigned c = 1) :
        M_Counter(int c = 1) :
            m_Count(c)
        {
        }
        int m_Count;
    };

    M_Counter* m_Counter;

    void Acquire(M_Counter* c)
    {
        m_Counter = c;
        if (c)
        {
            c->m_Count +=1;
        }
    }

    void Release()
    {
        if (m_Counter)
        {
            m_Counter->m_Count -=1;
            if (m_Counter->m_Count == 0)
            {
                delete m_RawPtr;
                delete m_Counter;
            }
        }
        m_Counter = 0;
        m_RawPtr = 0;
    }
};


template <typename X, typename Y> bool operator==
    (const RefPtr<X>& lptr, const RefPtr<Y>& rptr)
{
    return lptr.Get() == rptr.Get();
}

template <typename X, typename Y> bool operator==
    (const RefPtr<X>& lptr, Y* raw)
{
    return lptr.Get() == raw ;
}

template <typename X> bool operator==
    (const RefPtr<X>& lptr, long num)
{
    if (num == 0 && !lptr.IsValid())
    {
        return true;
    }
    else
    {
        return lptr == reinterpret_cast<X*>(num);
    }
}

template <typename X, typename Y> bool operator!=
    (const RefPtr<X>& lptr, const RefPtr<Y>& rptr)
{
    return (!operator== (lptr, rptr));
}

template <typename X, typename Y> bool operator!=
    (const RefPtr<X>& lptr, Y* raw)
{
    return (!operator== (lptr, raw));
}

template <typename X> bool operator!=
    (const RefPtr<X>& lptr, long num)
{
    return (!operator== (lptr, num));
}

#endif // _SMARTAUDIOPLAYER_REFPTR_H

