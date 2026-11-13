// The platform's thread-local variables (descriptors and template).
__thread int tlv_count = 5;
__thread int tlv_zero;

int tlv_next(void) { return tlv_count + tlv_zero++; }
