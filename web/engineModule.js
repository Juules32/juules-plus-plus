/*
    Engine Module Setup
*/
let setup, makeMoveStr, makeMove, engineMove, validTargets, validMove, isCheckmate

async function initializeEngine() {
    const engineModule = await Module() // Wait for the WebAssembly module to load

    setup = engineModule.cwrap('setup', 'string', null)
    makeMoveStr = engineModule.cwrap('make_move_str', 'string', ['string'])
    makeMove = engineModule.cwrap('make_move', 'string', ['number'])
    engineMove = engineModule.cwrap('engine_move', 'string', ['number', 'number'])
    validMove = engineModule.cwrap('valid_move', 'number', ['number', 'number', 'number'])
    validTargets = engineModule.cwrap('valid_targets', 'number', ['number', 'number'])
    isCheckmate = engineModule.cwrap('is_checkmate', 'number', null)
}

/*
    Util
*/
const pieceTypes = ["P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k"]

// Translation to api image format
const pieceTypeToAbbr = {
    "P": "wp",
    "N": "wn",
    "B": "wb",
    "R": "wr",
    "Q": "wq",
    "K": "wk",
    "p": "bp",
    "n": "bn",
    "b": "bb",
    "r": "br",
    "q": "bq",
    "k": "bk"
}

let pieceImages = {}
pieceTypes.forEach(pieceType => {
    const pieceImage = new Image()
    pieceImage.src = `https://images.chesscomfiles.com/chess-themes/pieces/wood/100/${pieceTypeToAbbr[pieceType]}.png`
    pieceImages[pieceType] = pieceImage
})

chessBoardImage = new Image()
chessBoardImage.src = `https://images.chesscomfiles.com/chess-themes/boards/blue/100.png`

moveSound = new Audio(`https://images.chesscomfiles.com/chess-themes/sounds/_MP3_/default/move-self.mp3`)

const fileNames = ["a", "b", "c", "d", "e", "f", "g", "h"]
const rankNames = ["8", "7", "6", "5", "4", "3", "2", "1"]

const white = 0
const black = 1
const both = 2
const neither = 3

/*
    State Management
*/
const startingTime = 180000
const increment = 0

let whiteTime = startingTime
let blackTime = startingTime

let whiteLocalTime = startingTime
let blackLocalTime = startingTime

let lastWhiteTimestamp = performance.now()
let lastBlackTimestamp = performance.now()

let interval

let playerSide = white
let sideToMove = white

let boardState = []
for (let i = 0; i < 8; i++) {
    boardState[i] = []
}

let hoveredFile = null
let hoveredRank = null
let hoveredSquare = null

let selectedSquare = null
let selectedFile = null
let selectedRank = null
let selectedPieceType = null

let moving = false

let winner = neither

let currentValidTargetsBitboard = 0
let validTargetSquares = []

function updateBoardState(fen) {
    selectedSquare = null
    selectedFile = null
    selectedRank = null
    selectedPieceType = null

    requestAnimationFrame(() => {
        parseFen(fen)
        if (isCheckmate()) {
            if (sideToMove == white) {
                winner = black
            }
            else {
                winner = white
            }
        }
        redraw()
    })
}

function setSide(side) {
    winner = neither
    playerSide = side
    whiteTime = startingTime
    blackTime = startingTime
    whiteLocalTime = startingTime
    blackLocalTime = startingTime
    clearInterval(interval)
    lastWhiteTimestamp = performance.now()
    lastBlackTimestamp = performance.now()
    startClock()
    updateBoardState(setup())
}

const whiteClock = document.getElementById('whiteClock')
const blackClock = document.getElementById('blackClock')

function startClock() {
    const updateFrequency = 33
    interval = setInterval(() => {
        if (sideToMove == white) {
            if (whiteLocalTime > 0) {
                whiteLocalTime -= updateFrequency
            }
            else {
                const newTimestamp = performance.now()
                whiteTime -= newTimestamp - lastWhiteTimestamp
                whiteLocalTime = whiteTime
                lastWhiteTimestamp = newTimestamp
                if (whiteTime <= 0) {
                    whiteLocalTime = 0
                    winner = black
                    redraw()
                }
            }
        }
        else {
            if (blackLocalTime > 0) {
                blackLocalTime -= updateFrequency
            }
            else {
                const newTimestamp = performance.now()
                blackTime -= newTimestamp - lastBlackTimestamp
                blackLocalTime = blackTime
                lastBlackTimestamp = newTimestamp
                if (blackTime <= 0) {
                    blackLocalTime = 0
                    winner = white
                    redraw()
                }
            }
        }
        updateClockDisplay()
    }, updateFrequency)
}

function formatMilliseconds(ms) {
    const minutes = Math.floor(ms / 60000); // Get whole minutes
    const seconds = Math.floor((ms % 60000) / 1000); // Get whole seconds
    const milliseconds = Math.floor((ms % 1000) / 10); // Get hundredths of a second

    const formattedMinutes = minutes.toString().padStart(2, '0'); // Ensure minutes are two digits
    const formattedSeconds = seconds.toString().padStart(2, '0'); // Ensure seconds are two digits
    const formattedMilliseconds = milliseconds.toString().padStart(2, '0'); // Ensure seconds are two digits

    if (minutes < 1) {
        // If less than a minute, format with two decimal places for milliseconds
        return `${formattedMinutes}:${formattedSeconds}.${formattedMilliseconds}`;
    } else {
        // If 1 minute or more, show minutes:seconds
        return `${formattedMinutes}:${formattedSeconds}`;
    }
}

function updateClockDisplay() {
    whiteClock.innerHTML = formatMilliseconds(whiteLocalTime)
    blackClock.innerHTML = formatMilliseconds(blackLocalTime)
}

function setValidTargetSquares(bitboard) {
    validTargetSquares = []
    const res = BigInt.asUintN(64, bitboard)
    for (let i = 0; i < 64; i++) {
        if (res & (BigInt(1) << BigInt(i))) {
            validTargetSquares.push(i)
        }
    }
}

function parseFen(fen) {

    const newTimestamp = performance.now()
    if (sideToMove == white) {
        whiteTime -= newTimestamp - lastWhiteTimestamp
        whiteLocalTime = whiteTime
    }
    else {
        blackTime -= newTimestamp - lastBlackTimestamp
        blackLocalTime = blackTime
    }
    lastWhiteTimestamp = newTimestamp
    lastBlackTimestamp = newTimestamp
    updateClockDisplay()

    const parts = fen.trim().split(' ')
    const position = parts[0]
    sideToMove = parts[1] == "w" ? white : black

    let i = 0
    for (char of position) {
        if (Number.isInteger(parseInt(char))) {
            for (let j = 0; j < parseInt(char); j++) {
                const file = i % 8
                const rank = Math.floor(i / 8)
                boardState[file][rank] = null
                i++
            }
        }
        else if (pieceTypes.includes(char)) {
            const file = i % 8
            const rank = Math.floor(i / 8)
            boardState[file][rank] = char
            i++
        }
    }

    if(winner == neither && (playerSide == neither || (playerSide != both && playerSide != sideToMove))) {
        requestAnimationFrame(() => {
            const fen = engineMove(sideToMove == white ? whiteTime : blackTime, increment)
            moveSound.pause()
            moveSound.currentTime = 0
            moveSound.play()
            updateBoardState(fen)
        })
    }
}

function tryMakingMove() {
    const move = validMove(selectedSquare, hoveredSquare, playerSide)
    if (move) {
        boardState[selectedFile][selectedRank] = null
        boardState[hoveredFile][hoveredRank] = selectedPieceType
        moveSound.play()
        updateBoardState(makeMove(move))
    }
}

/*
    Visuals
*/
const c = document.getElementById("board")
//Disables right click menu in canvas
c.addEventListener('contextmenu', event => event.preventDefault())
const ctx = c.getContext("2d")
ctx.font = "30px Arial"

function drawTargets() {
    ctx.fillStyle = "rgba(0, 0, 0, 0.15)"
    ctx.strokeStyle = "rgba(0, 0, 0, 0.15)"
    for (const validTarget of validTargetSquares) {
        const file = validTarget % 8
        const rank = Math.floor(validTarget / 8)
        
        if (boardState[file][rank] == null) {
            ctx.beginPath()
            ctx.arc(100 * file + 50, 100 * rank + 50, 25, 0, 2 * Math.PI)
            ctx.fill()
        }
        else {
            ctx.lineWidth = 8
            ctx.beginPath()
            ctx.arc(100 * file + 50, 100 * rank + 50, 46, 0, 2 * Math.PI)
            ctx.stroke()
        }
    }
}

function drawSelectedPiece(e) {
    if (selectedSquare != null && moving) {
        ctx.drawImage(pieceImages[selectedPieceType], e.offsetX - 50, e.offsetY - 50)
    }
}

function drawSelectedSquare(selectedFile, selectedRank) {
    ctx.fillStyle = "rgb(39, 176, 84)"
    ctx.beginPath()
    ctx.rect(100 * selectedFile, 100 * selectedRank, 100, 100)
    ctx.fill()
}

function redraw() {
    ctx.drawImage(chessBoardImage, 0, 0)
    if (selectedSquare != null) {
        drawSelectedSquare(selectedFile, selectedRank)
    }
    for (let file = 0; file < 8; file++) {
        for (let rank = 0; rank < 8; rank++) {
            const piece = boardState[file][rank]
            if (piece != null && (selectedSquare != rank * 8 + file || !moving)) {
                ctx.drawImage(pieceImages[piece], 100 * file, 100 * rank)
            }
        }
    }
    if (selectedSquare != null) {
        drawTargets()
    }

    if (winner != neither) {
        ctx.fillStyle = "rgba(0, 0, 0, 0.5)"
        ctx.rect(0, 0, 800, 800)
        ctx.fill()

        ctx.fillStyle = "white"
        ctx.fillText(`${winner == white ? "White" : "Black"} won!`, 326, 360)
    }
}

window.onload = () => {
    initializeEngine().then(() => {
        setSide(white)
    })
}

/*
    Event Listeners
*/
c.addEventListener('mousemove', e => {
    if (winner != neither) {
        return
    }

    hoveredFile = Math.floor(e.offsetX / 100)
    hoveredRank = Math.floor(e.offsetY / 100)
    hoveredSquare = hoveredRank * 8 + hoveredFile
    redraw()
    drawSelectedPiece(e)
})

c.addEventListener('mousedown', e => {
    if (winner != neither) {
        return
    }

    if (boardState[hoveredFile][hoveredRank] != null) {
        moving = true
        selectedSquare = hoveredSquare
        selectedFile = hoveredFile
        selectedRank = hoveredRank
        selectedPieceType = boardState[hoveredFile][hoveredRank]
        currentValidTargetsBitboard = validTargets(selectedSquare, playerSide)
        setValidTargetSquares(currentValidTargetsBitboard)
        ctx.drawImage(pieceImages[selectedPieceType], e.offsetX - 50, e.offsetY - 50)
    }
    else {
        if (selectedSquare != null) {
            tryMakingMove()
        }
        selectedSquare = null
        selectedFile = null
        selectedRank = null
        selectedPieceType = null
    }
    redraw()
    drawSelectedPiece(e)
})

c.addEventListener('mouseup', () => {
    if (winner != neither) {
        return
    }

    if (selectedSquare != null) {
        tryMakingMove()
    }
    moving = false
    redraw()
})
