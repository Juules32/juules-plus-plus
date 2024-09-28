/*
    Engine Module Setup
*/
let setup, makeMoveStr, makeMove, engineMove, validTargets, validMove

async function initializeEngine() {
    const engineModule = await Module() // Wait for the WebAssembly module to load

    setup = engineModule.cwrap('setup', 'string', null)
    makeMoveStr = engineModule.cwrap('make_move_str', 'string', ['string'])
    makeMove = engineModule.cwrap('make_move', 'string', ['number'])
    engineMove = engineModule.cwrap('engine_move', 'string', null)
    validMove = engineModule.cwrap('valid_move', 'number', ['number', 'number', 'number'])
    validTargets = engineModule.cwrap('valid_targets', 'number', ['number', 'number'])
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

const fileNames = ["a", "b", "c", "d", "e", "f", "g", "h"]
const rankNames = ["8", "7", "6", "5", "4", "3", "2", "1"]

const white = 0
const black = 1
const both = 2

/*
    State Management
*/

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

let currentValidTargetsBitboard = 0
let validTargetSquares = []

function updateBoardState(fen) {
    parseFen(fen)
    redraw()
}

function setSide(side) {
    playerSide = side
    updateBoardState(setup())
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

    if (playerSide != both && playerSide != sideToMove) {
        updateBoardState(engineMove())
    }
}

function tryMakingMove() {
    const move = validMove(selectedSquare, hoveredSquare, playerSide)
    if (move) {
        selectedSquare = null
        selectedFile = null
        selectedRank = null
        selectedPieceType = null
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
}

window.onload = () => {
    initializeEngine().then(() => {
        updateBoardState(setup())
    })
}

/*
    Event Listeners
*/
c.addEventListener('mousemove', e => {
    hoveredFile = Math.floor(e.offsetX / 100)
    hoveredRank = Math.floor(e.offsetY / 100)
    hoveredSquare = hoveredRank * 8 + hoveredFile
    redraw()
    drawSelectedPiece(e)
})

c.addEventListener('mousedown', e => {
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
    if (selectedSquare != null) {
        tryMakingMove()
    }
    moving = false
    redraw()
})
