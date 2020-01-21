class Integer {
private:
    int data;
public:
    //Integer(){}//added
    Integer(const int &value) : data(value) {}
    Integer(const Integer &other) : data(other.data) {}
    bool operator==(const Integer &t)
    {
        return data == t.data;
    }
};

