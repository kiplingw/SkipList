/*
    Copyright (C) 2024-2025 Cartesian Theatre. All rights reserved.
*/

// Multiple include protection...
#ifndef _SKIP_LIST_H_
#define _SKIP_LIST_H_

// Includes...

    // Standard C++ / POSIX system headers...
    #include <algorithm>
    #include <array>
    #include <cassert>
    #include <cmath>
    #include <cstddef>
    #include <functional>
    #include <iterator>
    #include <limits>
    #include <memory>
    #include <optional>
    #include <random>
    #include <tuple>
    #include <type_traits>
    #include <utility>

// Custom immutable forward iterator that iterates across a skip list.
//  Prior to C++17, it was encouraged to inherit from std::iterator
//  which would automatically populate our class with all type
//  definitions. This is discouraged since C++17...
template <typename NodeType>
class SkipListIterator
{
    // Public traits...
    public:

        // Some compilers or iterators will complain if we don't provide
        //  appropriate iterator traits. These tell the STL that our
        //  iterator is of the bidirectional iterator category and can
        //  iterates over Type values...

            // Signed integer that can be used to identify distance
            //  between iterators...
            using difference_type   = std::ptrdiff_t;

            // Category iterator belongs to...
            using iterator_category = std::forward_iterator_tag;

            // Type of object when iterator is dereferenced...
            using value_type        = typename NodeType::value_type;

            // Type of reference to the type iterated over...
            using reference         = value_type &;

            // Pointer to type iterated over...
            using pointer           = value_type *;

    // Public methods...
    public:

        // Default constructor...
        SkipListIterator() noexcept
          : m_CurrentNode(nullptr)
        {
        }

        // Construct pointing to the given node in the list...
        SkipListIterator(NodeType *CurrentNode) noexcept
          : m_CurrentNode(CurrentNode)
        {
        }

        // Dereference operator returns current key and value pair. This
        //  is const because the user modifying the key could change the
        //  sort order in the skip list...
        const reference operator*() const noexcept
        {
            return m_CurrentNode->GetKeyValue();
        }

        // Access operator returns pointer to key and value pair. This
        //  is const because the user modifying the key could change the
        //  sort order in the skip list...
        const pointer operator->() const noexcept
        {
            return &(m_CurrentNode->GetKeyValue());
        }

        // Prefix increment operator...
        SkipListIterator &operator++()
        {
            // Seek to the next node on bottom level...
            m_CurrentNode = m_CurrentNode->GetForwardPointer(0);

            // Return reference to updated iterator...
            return *this;
        }

        // Postfix increment operator...
        SkipListIterator operator++(int) noexcept
        {
            // Return previous state, incrementing our self...
            return std::exchange(*this, ++*this);
        }

        // Inequality operator. This is used in for loops where the
        //  iterator is compared against the end iterator. If they are
        //  unequal, it will continue iterating...
        bool operator!=(const SkipListIterator &RightHandSide) const noexcept
        {
            // It's true that the iterators are unequal if their
            //  positions differ...
            return m_CurrentNode != RightHandSide.m_CurrentNode;
        }

        // Equality operator...
        bool operator==(const SkipListIterator &RightHandSide) const noexcept
        {
            // It's true that the iterators are equal if their positions
            //  are...
            return m_CurrentNode == RightHandSide.m_CurrentNode;
        }

    // Protected attributes...
    protected:

        // Current node...
        NodeType   *m_CurrentNode;
};

// Skip list is a data structure discovered by William Pugh (1989) that can be
//  used in place of balanced trees. It uses probabilistic balancing. It works
//  well if the elements are inserted in random order, or, unlike a binary tree,
//  in sorted order too. It has the same asymptotic expected time bounds as a
//  binary tree, but is simpler to implement, faster, and uses less storage...
template
<
    typename    KeyType,                                                        /* Key type */
    typename    ValueType,                                                      /* Value type associated with key */
    typename    LessThanComparisonType = decltype(std::less<KeyType>()),        /* How to compare keys to each other */
    int         MaximumLevels = 16                                              /* Maximum number of levels, each indexed from [0, MaximumLevel) */
>
class SkipList
{
    // Protected forward declarations...
    protected:

        // Node type will be defined later...
        class NodeType;

    // Public types...
    public:

        // Key-value type...
        using KeyValueType      = std::pair<const KeyType, ValueType>;

        // Type alias for iterator...
        using iterator          = SkipListIterator<NodeType>;

        // Type alias for const iterator...
        using const_iterator    = iterator;

        // Type alias for how we count elements...
        using size_type         = std::size_t;

    // Public methods...
    public:

        // Constructor...
        explicit SkipList(
            const LessThanComparisonType &LessThanCompare = LessThanComparisonType())
          : m_Header(nullptr),
            m_End(nullptr),
            m_HighestLevel(0),
            m_LessThanComparison(LessThanCompare),
            m_RandomGenerator(m_RandomDevice()),                                /* Provide a constant instead for deterministic behaviour during debugging */
            m_Size(0)
        {
            // Allocate the head node...
            m_Header = new NodeType();

            // Allocate the terminal node...
            m_End = new NodeType();

            // The head node's forward pointers all point initially to the
            //  terminal node...
            m_Header->SetForwardPointers(m_End);
        }

        // Retrieve an iterator start...
        iterator begin() noexcept
        {
            // Should point to either the first node after the header, which
            //  might be the terminal node if there aren't any...
            return iterator(m_Header->GetForwardPointer(0));
        }

        // Retrieve a const iterator start...
        const_iterator begin() const noexcept
        {
            // Should point to either the first node after the header, which
            //  might be the terminal node if there aren't any...
            return const_iterator(m_Header->GetForwardPointer(0));
        }

        // Retrieve a const iterator start...
        const_iterator cbegin() const noexcept
        {
            // Should point to either the first node after the header, which
            //  might be the terminal node if there aren't any...
            return const_iterator(m_Header->GetForwardPointer(0));
        }

        // Retrieve an iterator end which is the next value after the last valid
        //  one...
        iterator end() noexcept
        {
            return iterator(m_End);
        }

        // Retrieve a const iterator end which is the next value after the last
        //  valid one...
        const_iterator end() const noexcept
        {
            return const_iterator(m_End);
        }

        // Retrieve a const iterator end which is the next value after the last
        //  valid one...
        const_iterator cend() const noexcept
        {
            return const_iterator(m_End);
        }

        // Clear all elements...
        void Clear()
        {
            // Track the current node we are about to delete, beginning with the
            //  head...
            NodeType *CurrentNode = m_Header->GetForwardPointer(0);

            // Keep walking the list until, deleting nodes, until we reach the
            //  terminal node...
            while(CurrentNode != m_End)
            {
                // Get the pointer to the next node, if any...
                NodeType *NextNode = CurrentNode->GetForwardPointer(0);

                // Delete the current node...
                delete CurrentNode;

                // Seek to the next one, if any...
                CurrentNode = NextNode;
            }

            // Update the header's forward pointers to point to the terminal...
            m_Header->SetForwardPointers(m_End);
            
            // Reset the highest level to only one... (we start counting at zero)
            m_HighestLevel = 0;

            // Reset the element count...
            m_Size = 0;
        }

        // Delete the given key and its associated value if the key exists.
        //  Return number of deleted elements, which should be either zero or
        //  one...
        size_type Delete(const KeyType &Key) noexcept
        {
            // Vector of forward pointers to maintain that is populated after
            //  the search is completed, but before performing the actual
            //  splice. Each level contains the rightmost node of that level or
            //  higher that is to the left of the location of the pending
            //  deletion...
            typename SkipList<KeyType, ValueType, LessThanComparisonType, MaximumLevels>::NodeType::ForwardPointersType
                UpdatedPointers{};

            // Start with the header node...
            NodeType *CurrentNode = m_Header;

            // Examine each level, from the highest level to the lowest for
            //  location to of node to delete...
            for(int CurrentLevel = m_HighestLevel;
                CurrentLevel >= 0;
              --CurrentLevel)
            {
                // Keep moving right on this level as far as we can without
                //  overshooting the location to insert or update the given
                //  key...
                while(IsLessThan(CurrentNode->GetForwardPointer(CurrentLevel)->GetKey(), Key))
                    CurrentNode = CurrentNode->GetForwardPointer(CurrentLevel);

                // Save the node on the left that will need to be spliced to the
                //  one on the right of the node to be deleted...
                UpdatedPointers.at(CurrentLevel) = CurrentNode;
            }

            // The next node is either the node to be deleted if it exists, or
            //  not...
            CurrentNode = CurrentNode->GetForwardPointer(0);

            // This node does not have the key we are looking for, so signal to
            //  caller it did not exist...
            if(CurrentNode->GetKey() != Key)
                return 0;

            // Otherwise we've found the node with the key we need to delete...
            else
            {
                // Splice pointers to point through the node we're about to
                //  delete to the next over...
                for(int CurrentLevel = 0; CurrentLevel <= m_HighestLevel; ++CurrentLevel)
                {
                    // If this node did not point to the one to be deleted, then
                    //  we can skip it...
                    if(UpdatedPointers.at(CurrentLevel)->GetForwardPointer(CurrentLevel) != CurrentNode)
                        break;

                    // Repair link between node on the left to the one to the
                    //  next one to the right of the node to be deleted...
                    UpdatedPointers.at(CurrentLevel)->SetForwardPointer(
                        CurrentLevel,
                        CurrentNode->GetForwardPointer(CurrentLevel));
                }

                // De-allocate the node...
                delete CurrentNode;
                CurrentNode = nullptr;

                // If we deleted the node with the highest level, adjust the
                //  list's highest level down to match the next highest...
                while(m_HighestLevel > 0 && !m_Header->GetForwardPointer(m_HighestLevel))
                  --m_HighestLevel;

                // Update the number of elements...
              --m_Size;

                // Signal to user deletion of a single element...
                return 1;
            }
        }

        // Get the number of elements...
        size_type GetSize() const noexcept { return m_Size; }

        // Insert the given key and value if it does not exist, or update its
        //  value if it does...
        void Insert(KeyType Key, ValueType Value)
        {
            // Vector of forward pointers to maintain that is populated after
            //  the search is completed, but before performing the actual
            //  splice. Each level contains the rightmost node of that level or
            //  higher that is to the left of the location of the pending
            //  insertion...
            typename SkipList<KeyType, ValueType, LessThanComparisonType, MaximumLevels>::NodeType::ForwardPointersType
                UpdatedPointers{};

            // Start with the header node...
            NodeType *CurrentNode = m_Header;

            // Examine each level, from the highest level to the lowest for
            //  location to update node or insert new one...
            for(int CurrentLevel = m_HighestLevel;
                CurrentLevel >= 0;
              --CurrentLevel)
            {
                // Keep moving right on this level as far as we can without
                //  overshooting the location to insert or update the given
                //  key...
                while(IsLessThan(CurrentNode->GetForwardPointer(CurrentLevel)->GetKey(), Key))
                    CurrentNode = CurrentNode->GetForwardPointer(CurrentLevel);

                // Check invariants...

                    // The node we are at should always be less than the key to
                    //  be inserted or updated...
                    assert(IsLessThan(CurrentNode->GetKey(), Key));

                    // The given key should also always be less than or equal to
                    //  the next one over...
                    assert(IsLessThanOrEqual(
                        Key,
                        CurrentNode->GetForwardPointer(CurrentLevel)->GetKey()));

                // Save the node on the left that will need to be spliced to the
                //  new node after it is created...
                UpdatedPointers.at(CurrentLevel) = CurrentNode;
            }

            // The next node is either the key whose value is to be updated, or
            //  the location to insert a new node at...
            CurrentNode = CurrentNode->GetForwardPointer(0);

            // This node has the given key, so update its value and we're
            //  done...
            if(CurrentNode->GetKey() == Key)
                CurrentNode->SetValue(std::move(Value));

            // Otherwise the key does not exist and we need to insert a new
            //  node...
            else
            {
                // Generate a random level for the proposed node...
                const int NewRandomLevel = GetRandomLevel();

                // The new random level is higher than the current highest
                //  level node in the list...
                if(NewRandomLevel > m_HighestLevel)
                {
                    // Remember to update the header's forward pointers above
                    //  the previous highest level...
                    for(int CurrentLevel = m_HighestLevel + 1;
                        CurrentLevel <= NewRandomLevel;
                      ++CurrentLevel)
                        UpdatedPointers.at(CurrentLevel) = m_Header;

                    // Remember that we've increased the highest level in the
                    //  list...
                    m_HighestLevel = NewRandomLevel;
                }

                // Allocate the new node and store its key and value...
                CurrentNode = new NodeType(
                    std::make_pair(
                        std::move(Key),
                        std::move(Value)));

                // Splice pointers...
                for(int CurrentLevel = 0; CurrentLevel <= m_HighestLevel; ++CurrentLevel)
                {
                    // ...for the new node to point to the node to its
                    //  right...
                    CurrentNode->SetForwardPointer(
                        CurrentLevel, 
                        UpdatedPointers.at(CurrentLevel)->GetForwardPointer(CurrentLevel));

                    // ...and for the preceeding nodes that need to point to
                    //  it...
                    UpdatedPointers.at(CurrentLevel)->SetForwardPointer(CurrentLevel, CurrentNode);
                }
                
                // Update node count...
              ++m_Size;
            }
        }

        // Search for the given key, returning an iterator to its key value pair
        //  if found, or the terminal node if not...
        iterator Search(const KeyType &SearchKey) const noexcept
        {
            // Start with the header node...
            NodeType *CurrentNode = m_Header;

            // Examine each level, from the highest level to the lowest...
            for(int CurrentLevel = m_HighestLevel;
                CurrentLevel >= 0;
              --CurrentLevel)
            {
                // Keep moving right on this level as far as we can without
                //  overshooting the location the search key should be located,
                //  if it exists...
                while(IsLessThan(CurrentNode->GetForwardPointer(CurrentLevel)->GetKey(), SearchKey))
                    CurrentNode = CurrentNode->GetForwardPointer(CurrentLevel);

                // Check invariants...

                    // The node we are at should always be less than the key to
                    //  search for...
                    assert(IsLessThan(CurrentNode->GetKey(), SearchKey));

                    // The search key should also always be less than or equal
                    //  to the next one over...
                    assert(IsLessThanOrEqual(
                        SearchKey,
                        CurrentNode->GetForwardPointer(CurrentLevel)->GetKey()));
            }

            // The next node is the search key, if it's present at all...
            CurrentNode = CurrentNode->GetForwardPointer(0);

            // If we found the search key, initialize and return an iterator
            //  to that node...
            if(CurrentNode->GetKey() == SearchKey)
                return SkipListIterator(CurrentNode);

            // Otherwise return an iterator pointing to the end to signal not
            //  found...
            else
                return SkipListIterator(m_End);
        }

        // Destructor...
       ~SkipList()
        {
            // Track the current node we are about to delete, beginning with the
            //  header...
            NodeType *CurrentNode = m_Header;

            // Keep walking the list until, deleting nodes, while there are
            //  some...
            while(CurrentNode)
            {
                // Get the pointer to the next node, if any...
                NodeType *NextNode = CurrentNode->GetForwardPointer(0);

                // Delete the current node...
                delete CurrentNode;

                // Seek to the next one, if any...
                CurrentNode = NextNode;
            }
        }

    // Protected types...
    protected:

        // Node type...
        class NodeType
        {
            // Public types...
            public:

				// Alias for the skip list's key value type...
				using value_type = KeyValueType;

            // Public methods...
            public:

                // Default constructor has default initialized values...
                NodeType()
                  : m_KeyValue(std::make_pair(KeyType(), ValueType())),
                    m_ForwardPointers{}
                {
                }

                // Construct by key and value...
                explicit NodeType(KeyValueType KeyValue)
                  : m_KeyValue(std::move(KeyValue)),
                    m_ForwardPointers{}
                {
                }

                // Get the forward pointer for the given level...
                NodeType *GetForwardPointer(const int Level) noexcept
                {
                    //return m_ForwardPointers.at(Level);
                    return m_ForwardPointers[Level];    /* Without bounds checking to allow callers to be const */
                }

                // Get the key...
                const KeyType &GetKey() const noexcept { return m_KeyValue.first; }

                // Get the key and value pair...
                KeyValueType &GetKeyValue() noexcept { return m_KeyValue; }
                const KeyValueType &GetKeyValue() const noexcept { return m_KeyValue; }

                // Get the level...
                int GetLevel() const noexcept
                {
                    // Variable to track level of node...
                    int Level = 0;

                    // Examine each forward pointer...
                    for(const auto Node : m_ForwardPointers)
                    {
                        // If it's non-null then we know we have a link to
                        //  another node...
                        if(Node)
                          ++Level;

                        // Otherwise we have reached the highest level of this
                        //  node...
                        else
                            break;
                    }

                    // Return node level...
                    return Level;
                }

                // Get the value...
                ValueType &GetValue() noexcept { return m_KeyValue.second; }
                const ValueType &GetValue() const noexcept { return m_KeyValue.second; }

                // Set the given level's forward pointer...
                void SetForwardPointer(
                    const int Level,
                    NodeType * const Node) noexcept
                {
                    m_ForwardPointers.at(Level) = Node;
                }

                // Set every forward pointer in the node to point to the
                //  given...
                void SetForwardPointers(NodeType * const Node) noexcept
                {
                    m_ForwardPointers.fill(Node);
                }

                // Set the value...
                void SetValue(ValueType NewValue) { m_KeyValue.second = std::move(NewValue); }

            // Public types...
            public:

                // Forward pointers type. By using an array instead of a linked
                //  list we can minimize cache misses...
                using ForwardPointersType = std::array<NodeType *, MaximumLevels>;

            // Protected attributes...
            protected:

                // Key and value pair...
                KeyValueType            m_KeyValue;

                // List of forward pointers, default initialized to nullptr...
                ForwardPointersType     m_ForwardPointers;
        };

    // Protected methods...
    protected:

        // Select a random level. Useful when creating a new node...
        int GetRandomLevel() noexcept
        {
            // We want a random distribution in the range of [0, 1)...
            std::uniform_real_distribution<float> RandomDistribution(0.0f, 1.0f);

            // Start with a single level...
            int CurrentLevel = 0;

            // Keep flipping a coin until it hits tails, but never increasing
            //  the level of the new node passed the maximum permissible...
            while(
                (RandomDistribution(m_RandomGenerator) < 0.5f) &&
                (CurrentLevel < (MaximumLevels - 1)))
            {
                // Increase the node's level...
              ++CurrentLevel;
            }

            // Return the proposed node's level...
            return CurrentLevel;
        }

        // Compare two keys for logical less than...
        bool IsLessThan(
            const KeyType &LeftHandSide,
            const KeyType &RightHandSide) const
        {
            // If the two keys refer to the same object, then axiomatically a
            //  key cannot be less than itself...
            if(&LeftHandSide == &RightHandSide)
                return false;

            // Treat the terminal node (largest possible key) as a special case
            //  that is always greater than anything else. If it's on the right
            //  side, then the expression is always true...
            else if(&RightHandSide == &(m_End->GetKey()))
                return true;

            // If the terminal node (largest possible key) is on the left side
            //  then it can never be less than anything else, and the expression
            //  is therefore always false...
            else if(&LeftHandSide == &(m_End->GetKey()))
                return false;

            // If the header (smallest possible key) is on the right side then
            //  it can never be less than anything else, and the expression is
            //  therefore always false...
            else if(&RightHandSide == &(m_Header->GetKey()))
                return false;

            // If the header (smallest possible key) is on the left side then it
            //  is always less than anything else, and the expression is
            //  therefore always true...
            else if(&LeftHandSide == &(m_Header->GetKey()))
                return true;

            // For all other scenarios use the user's comparison object...
            else
                return m_LessThanComparison(LeftHandSide, RightHandSide);
        }

        // Compare two keys for logical less than or equal to...
        bool IsLessThanOrEqual(
            const KeyType &LeftHandSide,
            const KeyType &RightHandSide) const
        {
            // Less than...
            if(IsLessThan(LeftHandSide, RightHandSide))
                return true;

            // Since we only have a user provided less than comparison object,
            //  we can also use it to check for equivalence because
            //  (A <= B) <--> (A < B) || (!(A < B) && !(B < A))
            //  If the second part of the dysjunction is true, then the keys
            //  must be logically equivalent...
            if(!IsLessThan(LeftHandSide, RightHandSide) &&
               !IsLessThan(RightHandSide, LeftHandSide))
                return true;

            // In all other scenarios the left hand side must be greater than
            //  the right...
            return false;
        }

    // Protected attributes...
    protected:

        // Header node...
        NodeType                       *m_Header;

        // Terminal node...
        NodeType                       *m_End;

        // Current highest level of any node, beginning counting levels at
        //  zero...
        int                             m_HighestLevel;

        // Less than comparison operator...
        LessThanComparisonType          m_LessThanComparison;

        // Random device to feed the generator...
        std::random_device              m_RandomDevice;

        // Mersenne Twister random generator from the random device...
        std::mt19937                    m_RandomGenerator;

        // Total number of elements...
        size_type                       m_Size;
};

// Multiple include protection...
#endif

