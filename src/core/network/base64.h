#pragma once
//
//  base64.h
//


#include <string>


class base64 {
    
public:
    
    base64(std::string msg="", bool packed=false){if(packed)m_data = msg;else{m_data = p_encode((const unsigned char *)msg.c_str(), msg.size());}};
    
    
    std::string encoded() const {return m_data;};
    std::string decoded() {return p_decode(m_data);};
    
    
    // Our Operator Overloads
    friend std::string& operator<< (std::string& left, base64& right){ left = right.p_decode(right.encoded()); return left;};
    friend base64 operator>> (std::string& left, base64& right){right = base64(left); return right;};
    
    inline bool operator==(const std::string& lhs){ if(lhs == p_decode(m_data)){return true;}else{return false;}};
    inline bool operator==(const base64& lhs){ if(lhs.encoded() == m_data){return true;}else{return false;}};
    
    
    
private:
    
    std::string p_encode(unsigned char const* , unsigned int len);
    std::string p_decode(std::string const& s);
    
    std::string m_data;
    
};