#include "ABuffer.h"
#include "assert0.h"

ABuffer::ABuffer(unsigned long long int capacity)
    : mRangeOffset(0),
      mInt32Data(0),
      mSampleNum(0),
      mPts(0),
      mOwnsData(true) {
    mData = (unsigned char *)malloc(capacity);
    if (mData == NULL) {
        mCapacity = 0;
        mRangeLength = 0;
    } else {
        mCapacity = capacity;
        mRangeLength = capacity;
    }
}

ABuffer::ABuffer(void *data, unsigned long long int capacity)
    : mData(data),
      mCapacity(capacity),
      mRangeOffset(0),
      mRangeLength(capacity),
      mInt32Data(0),
      mSampleNum(0),
      mPts(0),
      mOwnsData(false) {
}

ABuffer::~ABuffer() {
    if (mOwnsData) {
        if (mData != NULL) {
            free(mData);
            mData = NULL;
        }
    }
}

void ABuffer::setRange(unsigned long long int offset, unsigned long long int size) {
    assert0(offset <= mCapacity);
    assert0((offset + size) <= mCapacity);

    mRangeOffset = offset;
    mRangeLength = size;
}

void ABuffer::setSampleNum(int SampleNum){
    mSampleNum = SampleNum;
}

void ABuffer::setPts(long long int Pts){
    mPts = Pts;
}


