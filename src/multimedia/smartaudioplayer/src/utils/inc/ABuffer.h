#ifndef A_BUFFER_H_

#define A_BUFFER_H_
#include "ABase.h"

struct ABuffer {
    ABuffer(unsigned long long int capacity);
    ABuffer(void *data, unsigned long long int capacity);

    unsigned char *base() { return (unsigned char *)mData; }
    unsigned char *data() { return (unsigned char *)mData + mRangeOffset; }
    unsigned long long int capacity() const { return mCapacity; }
    unsigned long long int size() const { return mRangeLength; }
    unsigned long long int offset() const { return mRangeOffset; }

    void setRange(unsigned long long int offset, unsigned long long int size);
    void setSampleNum(int SampleNum);
    void setPts(long long int Pts);

    void setInt32Data(int data) {mInt32Data = data;}
    int int32Data() const {return mInt32Data;}
    int GetSamNum() const {return mSampleNum;}
    long long int GetPts() const {return mPts;}

    virtual ~ABuffer();

private:
    void *mData;
    unsigned long long int mCapacity;
    unsigned long long int mRangeOffset;
    unsigned long long int mRangeLength;   
    int  mInt32Data;
    int mSampleNum;
    long long int mPts;

    bool mOwnsData;

    DISALLOW_EVIL_CONSTRUCTORS(ABuffer);
};

#endif  // A_BUFFER_H_
