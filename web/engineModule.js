
// Engine Module
let test, setup, printGame, makeMove, findMove

async function initializeEngine() {
    const engineModule = await Module(); // Wait for the WebAssembly module to load

    test = engineModule.cwrap('test', 'string', ['string']);
    test('Testing testing function');

    setup = engineModule.cwrap('setup', 'string', null);
    printGame = engineModule.cwrap('print_game', null, null);
    makeMove = engineModule.cwrap('make_move', 'string', ['string']);
    findMove = engineModule.cwrap('find_move', 'string', null);
    
    console.log("Engine initialized");
}

function updateBoardState(fen) {
    parseFen(fen)
    redraw()
}

function reset() {
    updateBoardState(setup())
}

function playMove(move) {
    updateBoardState(makeMove(move))
}

function engineMove() {
    updateBoardState(findMove())
}

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

console.log(pieceImages["P"])

const fileNames = ["a", "b", "c", "d", "e", "f", "g", "h"]
const rankNames = ["8", "7", "6", "5", "4", "3", "2", "1"]


let boardState = []
for (let i = 0; i < 8; i++) {
    boardState[i] = []
}


let currentFile = null
let currentRank = null
let selectedPiece = null
let selectedFile = null
let selectedRank = null
let selectedPieceType = null
let moving = false
let side = "white"
let hoveredSquare = null

const c = document.getElementById("board");
const ctx = c.getContext("2d");
ctx.font = "30px Arial"

console.log(pieceImages[0])
function update() {
    ctx.drawImage(pieceImages[0], 50, 50)

}

window.onload = () => {
    initializeEngine().then(() => {
        reset()
    })
}

function parseFen(fen) {
    const parts = fen.trim().split(' ')
    const position = parts[0]
    side = parts[1]

    let i = 0;
    for (char of position) {
        if (Number.isInteger(parseInt(char))) {
            for (let j = 0; j < parseInt(char); j++) {
                let file = i % 8
                let rank = Math.floor(i / 8)
                boardState[file][rank] = null
                i++
            }
        }
        else if (pieceTypes.includes(char)) {
            let file = i % 8
            let rank = Math.floor(i / 8)
            boardState[file][rank] = char
            i++
        }
    }
    console.log(boardState)
}

function redraw() {
    ctx.drawImage(chessBoardImage, 0, 0)
    for (let file = 0; file < 8; file++) {
        for (let rank = 0; rank < 8; rank++) {
            const piece = boardState[file][rank]
            if (piece != null && (selectedPiece != rank * 8 + file || !moving)) {
                ctx.drawImage(pieceImages[piece], 100 * file, 100 * rank)
            }
        }
    }
}

c.addEventListener('mousemove', e => {
    currentFile = Math.floor(e.offsetX / 100)
    currentRank = Math.floor(e.offsetY / 100)
    hoveredSquare = currentRank * 8 + currentFile
    redraw()
    ctx.fillText(currentFile + " " + currentRank,  0, 50)
    if (selectedPiece != null && moving) {
        ctx.drawImage(pieceImages[selectedPieceType], e.offsetX - 50, e.offsetY - 50)
    }
})

c.addEventListener('mousedown', e => {
    if (boardState[currentFile][currentRank] != null) {
        console.log("piece")
        moving = true
        selectedPiece = hoveredSquare
        selectedFile = currentFile
        selectedRank = currentRank
        selectedPieceType = boardState[currentFile][currentRank]
        redraw()
        ctx.drawImage(pieceImages[selectedPieceType], e.offsetX - 50, e.offsetY - 50)
    }
})

c.addEventListener('mouseup', e => {
    if (selectedPiece != currentRank * 8 + currentFile) {
        const startFile = fileNames[selectedFile]
        const startRank = rankNames[selectedRank]
        const startSquare = startFile + startRank
        console.log(startSquare)

        const endFile = fileNames[currentFile]
        const endRank = rankNames[currentRank]
        const endSquare = endFile + endRank
        console.log(endSquare)
        // make move
        playMove(startSquare + endSquare)
        /*
        if fails:
        selectedPiece = null
        selectedFile = null
        selectedRank = null
        selectedPieceType = null
        */
    }
    moving = false
})
