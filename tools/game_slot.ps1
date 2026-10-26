<#
[Blur2PC] The machine-wide game launch queue for automated runs (every phase, every session).

The game allows one running copy (retail's BlurMutex): a copy started while another runs exits at
once. Sessions that each poll for a free moment race one another and a session relaunching in a
loop starves the rest. This script queues: it takes the named system mutex Global\Blur2PC.GameSlot
(a blocking wait, woken by the system when the holder releases it; no polling), then waits on the
handle of any game already running outside the queue (a user's game, a script that does not use
this file) until it exits, starts the game, waits for it to exit, and releases the slot.

    & <Blur1PC>\tools\game_slot.ps1 -Exe <path\Amax.exe> -ArgumentList '"--profile=x"' `
        -WorkingDirectory <dir> -TimeoutSeconds 600 -Owner "<session name>"

The caller sets the game's environment (BLUR2PC_AUTOMATION, ...) before calling; the game inherits
it. Output: an object with ExitCode, Seconds (run time), Waited (seconds in the queue), TimedOut.
A run that exceeds TimeoutSeconds is stopped (only the process this script started).
#>
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [string[]]$ArgumentList = @(),
    [string]$WorkingDirectory = (Split-Path -Parent $Exe),
    [int]$TimeoutSeconds = 600,
    [string]$Owner = "unknown"
)

$slot = New-Object System.Threading.Mutex($false, "Global\Blur2PC.GameSlot")
$queued = Get-Date
try { [void]$slot.WaitOne() } catch [System.Threading.AbandonedMutexException] { }	# a holder died: the slot is ours
try {
    # games not started through the queue: wait on their handles (the system signals the exit)
    foreach ($p in @(Get-Process Amax, AmaxPH, Blur -ErrorAction SilentlyContinue)) {
        try { $p.WaitForExit() } catch { }
    }
    $waited = ((Get-Date) - $queued).TotalSeconds
    $start = Get-Date
    $game = Start-Process -FilePath $Exe -ArgumentList $ArgumentList -WorkingDirectory $WorkingDirectory -PassThru
    $timedOut = -not $game.WaitForExit($TimeoutSeconds * 1000)
    if ($timedOut) { Stop-Process -Id $game.Id -Force; $game.WaitForExit() }
    [pscustomobject]@{
        ExitCode = $game.ExitCode
        Seconds  = [int]((Get-Date) - $start).TotalSeconds
        Waited   = [int]$waited
        TimedOut = $timedOut
        Owner    = $Owner
    }
}
finally {
    $slot.ReleaseMutex()
    $slot.Dispose()
}
