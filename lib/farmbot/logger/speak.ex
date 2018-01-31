defmodule Farmbot.Logger.Speak do
  use GenServer

  def start_link do
    GenServer.start_link(__MODULE__, [System.find_executable("espeak")], [name: __MODULE__])
  end

  def speak(text) do
    GenServer.call(__MODULE__, {:speak, text})
  end

  def init([exe]) do
    if exe do
      espeak = Port.open({:spawn_executable, exe}, [])
      os_pid = Port.info(espeak)[:os_pid] |> to_string() |> IO.inspect
      cpu_usage = "#{:code.priv_dir(:farmbot)}/cpu_usage"
      cpu_usage_port = Port.open({:spawn_executable, cpu_usage}, [:stream,
              :binary,
              :exit_status,
              :hide,
              :use_stdio,
              :stderr_to_stdout,
              args: ["#{os_pid}"]
              ])
      {:ok, %{espeak: espeak, cpu_usage: cpu_usage_port}}
    else
      IO.puts "no exe"
      {:ok, :no_exe}
    end
  end

  def handle_call({:speak, _}, _, :no_exe), do: {:reply, :ok, :no_exe}

  def handle_call({:speak, text}, _, state) do
    Port.command(state.espeak, text <> "\r\n")
    {:reply, :ok, state}
  end

  def handle_info({cpu_usage_port, {:data, data}}, %{cpu_usage: port} = state) when cpu_usage_port == port do
    IO.puts data
    {:noreply, state}
  end

  def handle_info(data, state) do
    {:noreply, state.abc}
  end

  def terminate(_, state) do
    System.cmd("killall", ["cpu_usage"])
    System.cmd("killall", ["espeak"])
  end
end
