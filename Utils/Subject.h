#pragma once

#include <vector>
#include <functional>
#include <memory>

#include "ReflectionUtils.h"

namespace leap
{
	class Subject
	{
	private:
		struct MemberFunctionWrapper
		{
			virtual ~MemberFunctionWrapper() = default;
			virtual void Call() const = 0;
			virtual unsigned int GetId() const = 0;
			virtual bool IsConst() const = 0;
		};

		template<typename T>
		struct ImplMemberFunctionWrapper final : public MemberFunctionWrapper
		{
			ImplMemberFunctionWrapper(T* _pThis, void(T::* _pFunction)())
				: pThis{ _pThis }, pFunction{ _pFunction }, id{ ReflectionUtils::GenerateTypenameHash<T>() }
			{
			}
			virtual ~ImplMemberFunctionWrapper() = default;

			unsigned int id{};
			T* pThis{};
			void(T::* pFunction)() {};

			virtual void Call() const override
			{
				(pThis->*pFunction)();
			}
			virtual unsigned int GetId() const override { return id; }
			virtual bool IsConst() const override { return false; }
		};

		template<typename T>
		struct ConstImplMemberFunctionWrapper final : public MemberFunctionWrapper
		{
			ConstImplMemberFunctionWrapper(T* _pThis, void(T::* _pFunction)() const)
				: pThis{ _pThis }, pFunction{ _pFunction }, id{ ReflectionUtils::GenerateTypenameHash<T>() }
			{
			}
			virtual ~ConstImplMemberFunctionWrapper() = default;

			unsigned int id{};
			T* pThis{};
			void(T::* pFunction)() const {};

			virtual void Call() const override
			{
				(pThis->*pFunction)();
			}
			virtual unsigned int GetId() const override { return id; }
			virtual bool IsConst() const override { return true; }
		};

	public:
		Subject() = default;

		template<typename T>
		void AddListener(T* pThis, void(T::* pFunction)())
		{
			std::unique_ptr<MemberFunctionWrapper> pFunctionWithThis{ std::make_unique<ImplMemberFunctionWrapper<T>>(pThis, pFunction) };
			m_pMemberFunctions.emplace_back(std::move(pFunctionWithThis));
		}
		template<typename T>
		void AddListener(T* pThis, void(T::* pFunction)() const)
		{
			std::unique_ptr<MemberFunctionWrapper> pFunctionWithThis{ std::make_unique<ConstImplMemberFunctionWrapper<T>>(pThis, pFunction) };
			m_pMemberFunctions.emplace_back(std::move(pFunctionWithThis));
		}
		void AddListener(void(*pCallback)())
		{
			m_Functions.push_back(pCallback);
		}
		template<typename T>
		void RemoveListener(T* pThis, void(T::* pFunction)())
		{
			constexpr unsigned int classId{ ReflectionUtils::GenerateTypenameHash<T>() };
			for (int i{ static_cast<int>(m_pMemberFunctions.size()) - 1 }; i >= 0; --i)
			{
				const auto it{ m_pMemberFunctions.begin() + i };

				const auto& pMemberFunction{ *it };
				if (pMemberFunction->IsConst()) continue;
				if (pMemberFunction->GetId() != classId) continue;

				ImplMemberFunctionWrapper<T>* pImpl{ static_cast<ImplMemberFunctionWrapper<T>*>(pMemberFunction.get()) };
				if (pImpl->pThis != pThis || pImpl->pFunction != pFunction) continue;

				m_pMemberFunctions.erase(it);
			}
		}
		template<typename T>
		void RemoveListener(T* pThis, void(T::* pFunction)() const)
		{
			constexpr unsigned int classId{ ReflectionUtils::GenerateTypenameHash<T>() };
			for (int i{ static_cast<int>(m_pMemberFunctions.size()) - 1 }; i >= 0; --i)
			{
				const auto it{ m_pMemberFunctions.begin() + i };

				const auto& pMemberFunction{ *it };
				if (!pMemberFunction->IsConst()) continue;
				if (pMemberFunction->GetId() != classId) continue;

				ConstImplMemberFunctionWrapper<T>* pImpl{ static_cast<ConstImplMemberFunctionWrapper<T>*>(pMemberFunction.get()) };
				if (pImpl->pThis != pThis || pImpl->pFunction != pFunction) continue;

				m_pMemberFunctions.erase(it);
			}
		}
		void RemoveListener(void(*pCallback)())
		{
			std::erase(m_Functions, pCallback);
		}

		void Notify()
		{
			for (const auto& pMemberFunction : m_pMemberFunctions)
			{
				pMemberFunction->Call();
			}
			for (const auto& function : m_Functions)
			{
				function();
			}
		}

	private:
		std::vector<void(*)()> m_Functions{};
		std::vector<std::unique_ptr<MemberFunctionWrapper>> m_pMemberFunctions;
	};

	template <class T>
	class TSubject
	{
	private:
		struct MemberFunctionWrapper
		{
			virtual ~MemberFunctionWrapper() = default;
			virtual void Call(const T& value) const = 0;
			virtual unsigned int GetId() const = 0;
			virtual bool IsConst() const = 0;
		};

		template<typename TClass>
		struct ImplMemberFunctionWrapper final : public MemberFunctionWrapper
		{
			ImplMemberFunctionWrapper(TClass* _pThis, void(TClass::* _pFunction)(const T&))
				: pThis{ _pThis }, pFunction{ _pFunction }, id{ ReflectionUtils::GenerateTypenameHash<TClass>() }
			{
			}
			virtual ~ImplMemberFunctionWrapper() = default;

			unsigned int id{};
			TClass* pThis{};
			void(TClass::* pFunction)(const T&) {};

			virtual void Call(const T& value) const override
			{
				(pThis->*pFunction)(value);
			}
			virtual unsigned int GetId() const override { return id; }
			virtual bool IsConst() const override { return false; }
		};

		template<typename TClass>
		struct ConstImplMemberFunctionWrapper final : public MemberFunctionWrapper
		{
			ConstImplMemberFunctionWrapper(TClass* _pThis, void(TClass::* _pFunction)(const T&) const)
				: pThis{ _pThis }, pFunction{ _pFunction }, id{ ReflectionUtils::GenerateTypenameHash<TClass>() }
			{
			}
			virtual ~ConstImplMemberFunctionWrapper() = default;

			unsigned int id{};
			TClass* pThis{};
			void(TClass::* pFunction)(const T&) const {};

			virtual void Call(const T& value) const override
			{
				(pThis->*pFunction)(value);
			}
			virtual unsigned int GetId() const override { return id; }
			virtual bool IsConst() const override { return true; }
		};

	public:
		TSubject() = default;

		template<typename TClass>
		void AddListener(TClass* pThis, void(TClass::* pFunction)(const T&))
		{
			std::unique_ptr<MemberFunctionWrapper> pFunctionWithThis{ std::make_unique<ImplMemberFunctionWrapper<TClass>>(pThis, pFunction) };
			m_pMemberFunctions.emplace_back(std::move(pFunctionWithThis));
		}
		template<typename TClass>
		void AddListener(TClass* pThis, void(TClass::* pFunction)(const T&) const)
		{
			std::unique_ptr<MemberFunctionWrapper> pFunctionWithThis{ std::make_unique<ConstImplMemberFunctionWrapper<TClass>>(pThis, pFunction) };
			m_pMemberFunctions.emplace_back(std::move(pFunctionWithThis));
		}
		void AddListener(void(*pCallback)(const T&))
		{
			m_Functions.push_back(pCallback);
		}
		template<typename TClass>
		void RemoveListener(TClass* pThis, void(TClass::* pFunction)(const T&))
		{
			constexpr unsigned int classId{ ReflectionUtils::GenerateTypenameHash<TClass>() };
			for (int i{ static_cast<int>(m_pMemberFunctions.size()) - 1 }; i >= 0; --i)
			{
				const auto it{ m_pMemberFunctions.begin() + i };

				const auto& pMemberFunction{ *it };
				if (pMemberFunction->IsConst()) continue;
				if (pMemberFunction->GetId() != classId) continue;

				ImplMemberFunctionWrapper<TClass>* pImpl{ static_cast<ImplMemberFunctionWrapper<TClass>*>(pMemberFunction.get()) };
				if (pImpl->pThis != pThis || pImpl->pFunction != pFunction) continue;

				m_pMemberFunctions.erase(it);
			}
		}
		template<typename TClass>
		void RemoveListener(TClass* pThis, void(TClass::* pFunction)(const T&) const)
		{
			constexpr unsigned int classId{ ReflectionUtils::GenerateTypenameHash<TClass>() };
			for (int i{ static_cast<int>(m_pMemberFunctions.size()) - 1 }; i >= 0; --i)
			{
				const auto it{ m_pMemberFunctions.begin() + i };

				const auto& pMemberFunction{ *it };
				if (!pMemberFunction->IsConst()) continue;
				if (pMemberFunction->GetId() != classId) continue;

				ConstImplMemberFunctionWrapper<TClass>* pImpl{ static_cast<ConstImplMemberFunctionWrapper<TClass>*>(pMemberFunction.get()) };
				if (pImpl->pThis != pThis || pImpl->pFunction != pFunction) continue;

				m_pMemberFunctions.erase(it);
			}
		}
		void RemoveListener(void(*pCallback)(const T&))
		{
			std::erase(m_Functions, pCallback);
		}

		void Notify(const T& data)
		{
			for (const auto& pMemberFunction : m_pMemberFunctions)
			{
				pMemberFunction->Call(data);
			}
			for (const auto& function : m_Functions)
			{
				function(data);
			}
		}
	private:
		std::vector<void(*)(const T&)> m_Functions{};
		std::vector<std::unique_ptr<MemberFunctionWrapper>> m_pMemberFunctions;
	};
}