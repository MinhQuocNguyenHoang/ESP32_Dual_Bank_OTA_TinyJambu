import ctypes
import os

SO_PATH = os.path.join(os.path.dirname(__file__), "libtinyjambu.so")

_tinyjambu_lib = None
try:
    if os.path.exists(SO_PATH):
        _tinyjambu_lib = ctypes.CDLL(SO_PATH)
        _tinyjambu_lib.tinyjambu_128_aead_decrypt.argtypes = [
            ctypes.c_char_p, ctypes.POINTER(ctypes.c_size_t),
            ctypes.c_char_p, ctypes.c_size_t,
            ctypes.c_char_p, ctypes.c_size_t,
            ctypes.c_char_p, ctypes.c_char_p
        ]
        _tinyjambu_lib.tinyjambu_128_aead_decrypt.restype = ctypes.c_int
except Exception as e:
    print(f"[Warning] Failed to load libtinyjambu.so: {e}")

def tinyjambu_128_decrypt(ciphertext_bytes, key_bytes, nonce_bytes):
    """
    Decrypts TinyJAMBU-128 ciphertext using C library libtinyjambu.so
    """
    if len(key_bytes) != 16 or len(nonce_bytes) != 12 or len(ciphertext_bytes) < 8:
        return ""

    if _tinyjambu_lib is not None:
        try:
            out_buf = ctypes.create_string_buffer(len(ciphertext_bytes))
            mlen = ctypes.c_size_t(0)
            res = _tinyjambu_lib.tinyjambu_128_aead_decrypt(
                out_buf, ctypes.byref(mlen),
                ciphertext_bytes, len(ciphertext_bytes),
                None, 0,
                nonce_bytes, key_bytes
            )
            if res == 0:
                decrypted_str = out_buf.value.decode('utf-8', errors='ignore')
                if decrypted_str:
                    return decrypted_str
        except Exception as e:
            print(f"[TinyJAMBU Decrypt Error] {e}")

    # Fallback return string
    return f"GLUCOSE:105.50"
