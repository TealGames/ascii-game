#include "pch.hpp"
#include "GameState.hpp"

GameState::GameState() {}

void GameState::SetHealth(const std::uint8_t newHealth)
{
	const std::uint8_t oldHealth = m_health;
	m_health = std::clamp(newHealth, std::uint8_t(0), MAX_HEALTH);
	m_OnHealthChanged.Invoke(oldHealth, m_health);
}
void GameState::SetHealthDelta(const std::uint8_t newHealth)
{
	SetHealth(m_health = newHealth);
}
std::uint8_t GameState::GetHealth() const
{
	return m_health;
}

void GameState::PauseGame()
{
	m_isPaused = true;
	m_OnGamePauseChange.Invoke(true);
}
void GameState::UnpauseGame()
{
	m_isPaused = false;
	m_OnGamePauseChange.Invoke(false);
}
void GameState::ToggleGamePause()
{
	if (m_isPaused) UnpauseGame();
	else PauseGame();
}
bool GameState::IsGamePaused() const
{
	return m_isPaused;
}