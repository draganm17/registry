template <typename ValueType>
class polymorphic_input_iterator
{
public:
    using value_type =        ValueType;
    using difference_type =   ptrdiff_t;
    using pointer =           const value_type*;
    using reference =         const value_type&;
    using iterator_category = std::input_iterator_tag;

public:
    virtual ~polymorphic_input_iterator() { }

    virtual bool operator==(const polymorphic_input_iterator&) const = 0;

    bool operator!=(const polymorphic_input_iterator& rhs) const { return !(*this == rhs); }

    virtual reference operator*() = 0;

    pointer operator->() { return &operator*(); }

    virtual polymorphic_input_iterator& operator++() = 0;

    polymorphic_input_iterator& operator++(int) { auto tmp = *this; ++*this; return tmp; }

public:
    virtual void swap(polymorphic_input_iterator&) noexcept = 0;
};

template <typename InputIt>
class string_sequence_iterator 
    : public polymorphic_input_iterator<string_view_type>
{
    InputIt m_it;
    string_view_type m_value;

public:
    string_sequence_iterator(InputIt it) : m_it(std::move(it)) { }

    bool operator==(const polymorphic_input_iterator<string_view_type>& other) const override
    { return m_it == dynamic_cast<const string_sequence_iterator&>(other).m_it; }

    reference operator*() override { return m_value = string_view_type(*m_it); }

    string_sequence_iterator& operator++() override { ++m_it; return *this; }

public:
    void swap(polymorphic_input_iterator& other) noexcept override
    {
        using std::swap;
        auto& other_ref = dynamic_cast<string_sequence_iterator&>(other);
        swap(m_it, other_ref.m_it);
        swap(m_value, other_ref.m_value);
    }
};

template <typename InputIt>
auto make_string_sequence_iterator(InputIt it) { return string_sequence_iterator<InputIt>(std::move(it)); }