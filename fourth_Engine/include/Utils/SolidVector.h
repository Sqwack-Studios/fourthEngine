#pragma once
#include <stdint.h>
#include <vector>
#include "include/Utils/Handle.h"
#include "include/Assert.h"

namespace fth
{
	template<typename T, typename S = uint32_t>
	class SolidVector
	{
	private: 
        using DataType = T;
        using Index = S;
		using ID = Handle<DataType, S>;


    public:
        bool occupied(ID id) const { assertIDbounds(); return m_forwardMap[id].occupied; }

        Index size() const { return Index(m_data.size()); }

        const T* data() const { return m_data.data(); }
        T* data() { return m_data.data(); }

        const T& at(Index index) const { assertIndex(index);  return m_data[index]; }
        T& at(Index index) { assertIndex(index);  return m_data[index]; }

        const T& operator[](ID id) const { assertId(id); return m_data[m_forwardMap[id]]; }
        T& operator[](ID id) { assertId(id); return m_data[m_forwardMap[id]]; }

        Index getIndexByID(ID id) { assertId(id); return m_forwardMap[id]; }

        ID queryNextHandle() const { return m_nextUnused; }

        ID insert(const T& value)
        {
            ID id = m_nextUnused;
            FTH_ASSERT_ENGINE(id <= m_forwardMap.size() && m_forwardMap.size() == m_occupied.size(), "Tried to insert new value"/*, but Next ID({0}) is > size({1})", id, m_forwardMap.size()*/);

            if (id.id == m_forwardMap.size())
            {
                m_forwardMap.push_back({ Index(m_forwardMap.size() + 1) });
                m_occupied.push_back(false);
            }

 /*           ForwardIndex& forwardIndex = m_forwardMap[id];
            m_nextUnused = forwardIndex.index;
            forwardIndex = { static_cast<uint32_t>(m_data.size()), true };*/
            FTH_ASSERT_ENGINE(!m_occupied[id], "ForwardIndex is occupied");


            m_nextUnused = m_forwardMap[id];
            m_forwardMap[id] = Index(m_data.size());
            m_occupied[id] = true;


            m_data.emplace_back(value);
            m_backwardMap.emplace_back(id);

            return id;
        }

        void erase(ID id)
        {
            FTH_ASSERT_ENGINE(id < m_forwardMap.size() && m_forwardMap.size() == m_occupied.size(), "Solid vector ID out of bounds");


            Index& forwardIndex = m_forwardMap[id];
            FTH_ASSERT_ENGINE(m_occupied[id], "Tried to erase index, but it's not occupied" /*{0} with ID {1}, but its not occupied", forwardIndex.index, id*/);

            m_data[forwardIndex] = std::move(m_data.back());
            m_data.pop_back();

            ID backwardIndex = m_backwardMap.back();

            m_backwardMap[forwardIndex] = backwardIndex;
            m_backwardMap.pop_back();

            m_forwardMap[backwardIndex] = forwardIndex;

            forwardIndex = m_nextUnused;
            m_occupied[id] = false;
            m_nextUnused = id;
        }

        void clear()
        {
            m_forwardMap.clear();
            m_backwardMap.clear();
            m_data.clear();
            m_occupied.clear();
            m_nextUnused = 0;
        }

        void reserve(size_t count)
        {
            m_forwardMap.reserve(count);
            m_data.reserve(count);
            m_backwardMap.reserve(count);
            m_occupied.reserve(count);
        }



    protected:
        std::vector<DataType> m_data;
        std::vector<Index> m_forwardMap;
        std::vector<ID> m_backwardMap;
        std::vector<bool> m_occupied;

        ID m_nextUnused = 0;

        void assertId(ID id) const
        {
            assertIDbounds(id);
            assertIDoccupation(id);
        }

        void assertIDbounds(ID id) const
        {
            FTH_ASSERT_ENGINE(id <= m_forwardMap.size() && m_forwardMap.size() == m_occupied.size(), "Solid vector ID out of bounds");
        }

        void assertIDoccupation(ID id) const
        {
            FTH_ASSERT_ENGINE(m_occupied[id], "Solid vector ID not occupied");
        }

        void assertIndex(uint32_t index) const
        {
            FTH_ASSERT_ENGINE(index < m_data.size(), "Solid vector index out of bounds");
        }

	};
}