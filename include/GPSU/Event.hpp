#pragma once

#include <functional>
#include <unordered_map>

namespace GPSU
{
	template<typename... TArgs>
	class Event
	{
	public:
		using THandler = std::function<void(TArgs...)>;

		class Private
		{
		public:
			inline Private() {}
			Private(const Private&) = delete;

			inline void operator()(TArgs... args) const { for (const auto& pair : _handlers) pair.second(args...); }
			inline Private& operator +=(const THandler& handler) { _handlers[++_last] = handler; return *this; }
			inline int Add(const THandler& handler) { _handlers[++_last] = handler; return _last; }
			inline void Remove(int id) { _handlers.erase(_handlers.begin() + id); }
			inline Event operator*() { return *this; }

		private:
			std::unordered_map<int, THandler> _handlers;
			int _last = -1;
		};

		inline Event(Private& priv) : _private(priv) {}
		inline Event& operator +=(const THandler& handler) { _private += handler; return *this; }
		inline int Add(const THandler& handler) { return _private.Add(handler); }
		inline void Remove(int id) { _private.Remove(id); }

	private:
		Private& _private;
	};
}
