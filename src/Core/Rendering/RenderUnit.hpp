#pragma once
#include "Core/Rendering/RenderingBackend.hpp"

namespace Rendering
{
	struct RenderBatch
	{
		Shader* m_Shader = nullptr;
		Texture* m_Texture = nullptr;

		size_t m_VertexStartIndex = -1;
		size_t m_VertexCount = 0;

		size_t m_IndicesStartIndex = -1;
		size_t m_IndicesCount = 0;

		size_t m_InstanceStartIndex = -1;
		size_t m_InstanceCount = 0;

		std::string ToString() const;
	};
	using BatchHash = std::uint64_t;
	using BatchIndex = std::uint8_t;
	constexpr BatchIndex INVALID_BATCH_INDEX = -1;
	struct BatchKey
	{
		std::uint16_t m_ShaderId;
		std::uint16_t m_TextureId;
		std::uint32_t m_VertexCount;
	};

	template<typename TVertex, typename TInstance>
	class RenderUnit
	{
	private:
		VertexLayout* m_layout;
		std::unordered_map<BatchHash, size_t> m_hashToBatchIndex;
	public:
		std::vector<RenderBatch> m_Batches;

		//TODO; we have way too many vectors, maybe make them static compile time arrays instead
		std::vector<TVertex> m_CpuVertices;
		std::vector<IndexType> m_CpuIndices;
		std::vector<TInstance> m_CpuInstances;

		IndexBuffer m_IndexBufferHandle;
		VertexBuffer m_VertexBufferHandle;
		VertexBuffer m_InstanceBufferHandle;
	private:
		size_t CalculateBatchHash(const Shader& shader, const Texture* texture, std::uint32_t totalVertices) const
		{
			const BatchKey batchKey = BatchKey(shader.GetId(), texture==nullptr? INVALID_OBJ_ID : texture->GetInfo().m_Id, totalVertices);
			return std::hash<BatchHash>{}(*reinterpret_cast<const BatchHash*>(&batchKey));
		}
		size_t CalculateBatchHash(const RenderBatch& batch) const
		{
			ENGINE_ASSERT(batch.m_Shader != nullptr, "RenderUnit attempted to calculate batch hash for: {} "
				"but it has no shader which is not allowed", batch.ToString());
			return CalculateBatchHash(*batch.m_Shader, batch.m_Texture, batch.m_VertexCount);
		}

	public:
		RenderUnit(VertexLayout& layout)
			: m_Batches(), m_layout(&layout), m_CpuVertices(), m_CpuIndices(), m_CpuInstances(),
			m_IndexBufferHandle(), m_VertexBufferHandle(), m_InstanceBufferHandle()
		{

		}

		void Init(const size_t vertexCount, const int indexMaxCount, const int instanceMaxCount)
		{
            m_CpuVertices.reserve(vertexCount);
			m_VertexBufferHandle = Backend::CreateVertexBuffer(nullptr, sizeof(TVertex), vertexCount, VertexAttributeAdvance::Vertex);
			if (indexMaxCount > 0)
			{
				const bool layoutBoundOnCall = m_layout->IsBoundActive();
				//NOTE: we make sure to bind the layout because the index buffer 
				//link to the vertex layout is implicit and relies on active bound layout
				if (!layoutBoundOnCall) m_layout->BindActive();

                m_CpuIndices.reserve(indexMaxCount);
				m_IndexBufferHandle = Backend::CreateIndexBuffer(nullptr, indexMaxCount);

				if (!layoutBoundOnCall) m_layout->UnbindActive();
			}
			if (instanceMaxCount > 0)
			{
                m_CpuInstances.reserve(instanceMaxCount);
				m_InstanceBufferHandle = Backend::CreateVertexBuffer(nullptr, sizeof(TInstance), instanceMaxCount, VertexAttributeAdvance::Instance);
			}
		}

		RenderBatch& CreateBatch(Shader& shader, Texture* texture)
		{
			return m_Batches.emplace_back(&shader, texture);
		}
		RenderBatch* TryGetBatch(const Shader& shader, const Texture* texture, std::uint32_t vertexCount)
		{
			const size_t hash = CalculateBatchHash(shader, texture, vertexCount);
			auto it = m_hashToBatchIndex.find(hash);

			if (it == m_hashToBatchIndex.end())
				return nullptr;
			return &(m_Batches[it->second]);
		}

        void AddVertexToBatch(RenderBatch& batch, const TVertex& vertex)
        {
            m_CpuVertices.emplace_back(vertex);
            if (batch.m_VertexCount == 0)
                batch.m_VertexStartIndex = m_CpuVertices.size() - 1;

            batch.m_VertexCount++;
        }
        void AddVerticesToBatch(RenderBatch& batch, const TVertex* vertexArray, const size_t vertexSize)
        {
            //NOTE: we do this before the insertion since start index is index greater than current last index
			if (batch.m_VertexCount == 0)
				batch.m_VertexStartIndex = m_CpuVertices.size();

            m_CpuVertices.insert(m_CpuVertices.end(), vertexArray, vertexArray + vertexSize);
            batch.m_VertexCount += vertexSize;
        }

		void AddIndicesToBatch(RenderBatch& batch, const std::array<IndexType, 3>& arr)
		{
			if (batch.m_IndicesCount == 0)
				batch.m_IndicesStartIndex = GetIndexCount();

			m_CpuIndices.push_back(batch.m_VertexStartIndex + arr[0]);
			m_CpuIndices.push_back(batch.m_VertexStartIndex + arr[1]);
			m_CpuIndices.push_back(batch.m_VertexStartIndex + arr[2]);

			batch.m_IndicesCount += arr.size();
		}
		void AddIndicesToBatch(RenderBatch& batch, const IndexType* indexArray, const size_t indicesSize)
		{
			if (batch.m_IndicesCount == 0)
				batch.m_IndicesStartIndex = m_CpuIndices.size();

			for (int i = 0; i < indicesSize; i++)
			{
				m_CpuIndices.push_back(batch.m_VertexStartIndex + indexArray[i]);
			}

			batch.m_IndicesCount += indicesSize;
		}

        template<typename... TArgs>
        TInstance& AddInstanceDataToBatch(RenderBatch& batch, TArgs&&... args)
        {
			TInstance& createdInstance = m_CpuInstances.emplace_back(std::forward<TArgs>(args)...);
            if (batch.m_InstanceCount == 0)
                batch.m_InstanceStartIndex = m_CpuInstances.size() - 1;

            batch.m_InstanceCount++;
            return createdInstance;
        }
		void FinishBatch(RenderBatch& batch)
		{
			ENGINE_ASSERT(batch.m_InstanceCount == 1, 
				"Attempted to finish a batch which has more than one instances (meaning it is already finished in setup)");
			m_hashToBatchIndex.emplace(CalculateBatchHash(batch), m_Batches.size() - 1);
		}

		void ClearAll()
		{
			m_CpuVertices.clear();
			m_CpuIndices.clear();
			m_CpuInstances.clear();
			m_Batches.clear();
			m_hashToBatchIndex.clear();
		}

		TVertex* GetVertexMemPointer() { return &m_CpuVertices[0]; }
		IndexType* GetIndexMemPointer() { return &m_CpuIndices[0]; }
		TInstance* GetInstanceMemPointer() { return &m_CpuInstances[0]; }

		size_t GetVertexCount() const { return m_CpuVertices.size(); }
		size_t GetIndexCount() const { return m_CpuIndices.size(); }
		size_t GetInstanceCount() const { return m_CpuInstances.size(); }

		std::string ToStringBatches() const
		{
			std::string result = std::format("TOTAL({})\n ", m_Batches.size());
			for (const auto& batch : m_Batches)
			{
				result += "\nBatch:" + batch.ToString() + "\n";
			}
			return result;
		}
	};
}